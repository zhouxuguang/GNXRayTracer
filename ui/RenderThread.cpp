#include "RenderThread.h"
#include "DebugText.h"
#include <QTime>

#include "RenderStatus.h"

#include <stdlib.h>
#include <time.h>

#include "core/Interaction.h"
#include "core/Spectrum.h"
#include "core/Scene.h"
#include "core/Integrator.h"
#include "accelerator/BVHAccel.h"
#include "camera/Perspective.h"
#include "samplers/HaltonSampler.h"
#include "lights/PointLight.h"

#include "core/Medium.h"
#include "media/HomogeneousMedium.h"
#include "media/GridDensityMedium.h"

#include "integrators/WhittedIntegrator.h"
#include "integrators/PathIntegrator.h"

#include "MaterialList.h"
#include "ModelList.h"
#include "Utils.h"

#include <omp.h>

using namespace pbr;


RenderThread::RenderThread()
{
	paintFlag = false;
	renderFlag = false;
}

RenderThread::~RenderThread()
{
	
}

void RenderThread::run()
{
	emit PrintString((char*)"Prepared to Render");

	ClockRandomInit();

	int WIDTH = 500;
	int HEIGHT = 500;

    emit PrintString((char*)"Init FrameBuffer");
    m_pFramebuffer->bufferResize(WIDTH, HEIGHT);
    
    //相机参数
    //Point3f eye(0.f, 2.f, 6.f), look(0.f, 0.f, 0.0f);
    Vector3f up(0.0f, 1.0f, 0.0f);
    
    Point3f eye(0.f, 0.f, 5.0f), look(0.f, 0.f, 0.0f);
    Transform lookat = LookAt(eye, look, up);

    Transform Camera2WorldStart = Inverse(lookat);
    Transform Camera2WorldEnd = Camera2WorldStart;
    AnimatedTransform animatedTrans(&Camera2WorldStart, 0.0f, &Camera2WorldEnd, 1.0f);
    std::shared_ptr<const Camera> camera = std::shared_ptr<Camera>(CreatePerspectiveCamera(WIDTH, HEIGHT, animatedTrans));
    
    //初始化材质
    emit PrintString((char*)"Init Material");
    std::shared_ptr<Material> dragonMaterial;
    std::shared_ptr<Material> whiteWallMaterial;
    std::shared_ptr<Material> redWallMaterial;
    std::shared_ptr<Material> blueWallMaterial;
    std::shared_ptr<Material> whiteLightMaterial;
    std::shared_ptr<Material> mirrorMaterial;
    {
        Spectrum whiteColor; whiteColor[0] = 0.91; whiteColor[1] = 0.91; whiteColor[2] = 0.91;
        Spectrum dragonColor; dragonColor[0] = 0.2; dragonColor[1] = 0.8; dragonColor[2] = 0.2;
        Spectrum redWallColor; redWallColor[0] = 0.9; redWallColor[1] = 0.1; redWallColor[2] = 0.17;
        Spectrum blueWallColor; blueWallColor[0] = 0.14; blueWallColor[1] = 0.21; blueWallColor[2] = 0.87;
        std::shared_ptr<Texture<Spectrum>> KdDragon = std::make_shared<ConstantTexture<Spectrum>>(dragonColor);
        std::shared_ptr<Texture<Spectrum>> KrDragon = std::make_shared<ConstantTexture<Spectrum>>(dragonColor);
        std::shared_ptr<Texture<Spectrum>> KdWhite = std::make_shared<ConstantTexture<Spectrum>>(whiteColor);
        std::shared_ptr<Texture<Spectrum>> KdRed = std::make_shared<ConstantTexture<Spectrum>>(redWallColor);
        std::shared_ptr<Texture<Spectrum>> KdBlue = std::make_shared<ConstantTexture<Spectrum>>(blueWallColor);
        //std::shared_ptr<Texture<float>> sigma = std::make_shared<ConstantTexture<float>>(0.0f);
        std::shared_ptr<Texture<float>> sigma = std::make_shared<ConstantTexture<float>>(60.0f);
        std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);

        dragonMaterial = std::make_shared<MatteMaterial>(KdDragon, sigma, bumpMap);
        //dragonMaterial = getPurplePlasticMaterial();
        //dragonMaterial = getYelloMetalMaterial();
        //dragonMaterial = getWhiteGlassMaterial();

        whiteWallMaterial = std::make_shared<MatteMaterial>(KdWhite, sigma, bumpMap);
        redWallMaterial = std::make_shared<MatteMaterial>(KdRed, sigma, bumpMap);
        blueWallMaterial = std::make_shared<MatteMaterial>(KdBlue, sigma, bumpMap);

        whiteLightMaterial = std::make_shared<MatteMaterial>(KdWhite, sigma, bumpMap);
        mirrorMaterial = std::make_shared<MirrorMaterial>(KrDragon, bumpMap);
    }
    
    //参与介质
    //根据自己的喜好进行调整
    HomogeneousMedium homogeneousMedium(2.4, 1.4, 0.5); //该接口给我们的模型使用
    MediumInterface mediumDragon(&homogeneousMedium, nullptr);

    
    Transform tri_Object2World , tri_World2Object;
    
    std::vector<std::shared_ptr<Primitive>> prims;
    
    //AddFloor(prims, whiteWallMaterial);
    
    emit PrintString((char*)"Init Mesh...");
    {
        AddModel(prims, dragonMaterial);
    }
    
    emit PrintString((char*)"Init Cornell Box...");
    {
        AddCornell(prims, redWallMaterial, blueWallMaterial, whiteWallMaterial);
    }
    
    // 光源列表
    std::vector<std::shared_ptr<Light>> lights;
    
    //初始化面光源
    emit PrintString((char*)"Init AreaLight");
    {
        AddAreaLight(prims, lights, dragonMaterial);
    }
    
    
    //聚光灯
    //AddSpotLight(lights);
    
    //平行光
    //AddDistLight(lights);
    
    
    //构造环境光源
    AddSkyLight(lights);
    
    
    //构造无限远面光源
//    emit PrintString((char*)"Init InfiniteLight...");
//    {
//        AddInfLight(lights);
//    }
    
    emit PrintString((char*)"Init worldScene");
    std::unique_ptr<Scene> worldScene = std::make_unique<Scene>(std::make_shared<BVHAccel>(prims, 1), lights);
    
    emit PrintString((char*)"Init Sampler");
    Bounds2i imageBound(Point2i(0, 0), Point2i(WIDTH , HEIGHT));
    std::shared_ptr<HaltonSampler> sampler = std::make_unique<HaltonSampler>(32, imageBound, false);
    //std::shared_ptr<ClockRandSampler> sampler = std::make_unique<ClockRandSampler>(64, imageBound);
    
    Bounds2i ScreenBound(Point2i(0, 0), Point2i(WIDTH, HEIGHT));
    std::shared_ptr<Integrator> integrator = std::make_shared<WhittedIntegrator>(5, camera, sampler, ScreenBound, m_pFramebuffer);
    //std::shared_ptr<Integrator> integrator = std::make_shared<PathIntegrator>(15, camera, sampler, ScreenBound, m_pFramebuffer, 1.f, "spatial");
    
    
    emit PrintString((char*)"Start Rendering");
	int renderCount = 0;
	while (renderFlag) {
        QElapsedTimer t;
		t.start();

		//emit PrintString("Rendering");
        double frameTime;
        integrator->Render(*worldScene, frameTime);

        g_RenderStatus.setDataChanged("Performance", "One Frame Time", QString::number(frameTime), "");
        g_RenderStatus.setDataChanged("Performance", "Frame pre second", QString::number(1.0f / (float)frameTime), "");

		emit PaintBuffer(m_pFramebuffer->getUCbuffer(), WIDTH, HEIGHT, 4);
			
		while (t.elapsed() < 1);

        showMemoryInfo();
	}
	
}


