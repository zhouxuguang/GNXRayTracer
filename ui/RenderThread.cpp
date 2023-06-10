#include "RenderThread.h"
#include "DebugText.h"
#include <QTime>

#include "RenderStatus.h"

#include <stdlib.h>
#include <time.h>

#include "shape/Triangle.h"
#include "shape/plyRead.h"
#include "core/Interaction.h"
#include "core/Spectrum.h"
#include "core/Scene.h"
#include "core/Integrator.h"
#include "accelerator/BVHAccel.h"
#include "camera/Perspective.h"
#include "samplers/HaltonSampler.h"
#include "core/Material.h"
#include "materials/MatteMaterial.h"
#include "materials/MirrorMaterial.h"
#include "core/Texture.h"
#include "textures/ConstantTexture.h"
#include "lights/PointLight.h"
#include "lights/DiffuseAreaLight.h"
#include "lights/SkyBoxLight.h"
#include "integrators/WhittedIntegrator.h"

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
    
    //初始化材质
    emit PrintString((char*)"Init Material");
    Spectrum floorColor; floorColor[0] = 0.2; floorColor[1] = 0.3; floorColor[2] = 0.9;
    Spectrum dragonColor; dragonColor[0] = 0.2; dragonColor[1] = 0.7; dragonColor[2] = 0.2;
    std::shared_ptr<Texture<Spectrum>> KdDragon = std::make_shared<ConstantTexture<Spectrum>>(dragonColor);
    std::shared_ptr<Texture<Spectrum>> KdFloor = std::make_shared<ConstantTexture<Spectrum>>(floorColor);
    
    Spectrum mirrorColor(1.0f);
    std::shared_ptr<Texture<Spectrum>> KrMirror = std::make_shared<ConstantTexture<Spectrum>>(dragonColor);
    std::shared_ptr<Texture<Float>> sigma = std::make_shared<ConstantTexture<Float>>(0.0f);
    std::shared_ptr<Texture<Float>> bumpMap = std::make_shared<ConstantTexture<Float>>(0.0f);
    //材质
    std::shared_ptr<Material> dragonMaterial = std::make_shared<MatteMaterial>(KdDragon, sigma, bumpMap);
    std::shared_ptr<Material> floorMaterial = std::make_shared<MatteMaterial>(KdFloor, sigma, bumpMap);
    std::shared_ptr<Material> whiteLightMaterial = std::make_shared<MatteMaterial>(KdFloor, sigma, bumpMap);
    std::shared_ptr<Material> mirrorMaterial = std::make_shared<MirrorMaterial>(KrMirror, bumpMap);

    
    Transform tri_Object2World , tri_World2Object;
    
    std::vector<std::shared_ptr<Primitive>> prims;
    
    //地板
    int nTrianglesFloor = 2;
    int vertexIndicesFloor[6] = { 0 ,1 ,2 ,3 ,4 ,5 };
    int nVerticesFloor = 6;
    const float yPos_Floor = -2.0;
    Point3f P_Floor[6] = {
        Point3f(-6.0, yPos_Floor, 6.0) ,Point3f(6.0, yPos_Floor, 6.0) , Point3f(-6.0, yPos_Floor, -6.0) ,
        Point3f(6.0, yPos_Floor, 6.0) ,Point3f(6.0, yPos_Floor, -6.0), Point3f(-6.0, yPos_Floor, -6.0)
    };
    
    Transform tri_Object2World2, tri_World2Object2;
    std::shared_ptr<TriangleMesh> meshFloor = std::make_shared<TriangleMesh>(tri_Object2World2, nTrianglesFloor, vertexIndicesFloor,
    nVerticesFloor, P_Floor, nullptr, nullptr, nullptr, nullptr);
    
    std::vector<std::shared_ptr<Shape>> trisFloor;
    for (int i = 0; i < nTrianglesFloor; ++i)
    {
        trisFloor.push_back(std::make_shared<Triangle>(&tri_Object2World2, &tri_World2Object2, false, meshFloor, i));
    }
    
    //将物体填充到基元
    for (int i = 0; i < nTrianglesFloor; ++i)
    {
        prims.push_back(std::make_shared<GeometricPrimitive>(trisFloor[i], floorMaterial, nullptr));
    }

    
    tri_Object2World = Translate(Vector3f(0.0, -2.5, 0.0)) * tri_Object2World;
    tri_World2Object = Inverse(tri_Object2World);

    //E:\code\model
#ifdef _WIN32
    plyInfo* plyi = new plyInfo("E:\\code\\model\\dragon.3d");
#else
    plyInfo* plyi = new plyInfo("/Users/zhouxuguang/work/opensource/pbrt/pdf/模型文件-1/dragon.3d");
#endif // _WIN32

    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(tri_Object2World, plyi->nTriangles, plyi->vertexIndices, plyi->nVertices, plyi->vertexArray, nullptr, nullptr, nullptr, nullptr);
    std::vector<std::shared_ptr<Shape>> tris;
    tris.reserve(plyi->nTriangles);
    
    //从mesh转为Shape类型，该代码可以从triangle .cpp文件里找到 tris.reserve(nTriangles);
    for (int i = 0; i < plyi->nTriangles; ++i)
    {
        tris.push_back(std::make_shared<Triangle>(&tri_Object2World, &tri_World2Object, false , mesh, i));
    }
    
    prims.reserve(plyi->nTriangles);
    for (int i = 0; i < plyi->nTriangles; ++i)
    {
        prims.push_back(std::make_shared<GeometricPrimitive>(tris[i], dragonMaterial, nullptr));
    }
    
    //相机参数
    Point3f eye(-4.0f, 1.f, -4.0f), look(0.0, 0.0, 0.0f);
    Vector3f up(0.0f, 1.0f, 0.0f);
    Transform lookat = LookAt(eye, look, up);

    Transform Camera2World = Inverse(lookat);
    std::shared_ptr<const Camera> camera = std::shared_ptr<Camera>(CreatePerspectiveCamera(WIDTH, HEIGHT, Camera2World));
    
    
    
    emit PrintString((char*)"Init Sampler");
    Bounds2i imageBound(Point2i(0, 0), Point2i(WIDTH , HEIGHT));
    //std::shared_ptr<HaltonSampler> sampler = std::make_unique<HaltonSampler>(8, imageBound, false);
    std::shared_ptr<ClockRandSampler> sampler = std::make_unique<ClockRandSampler>(8, imageBound);
    
    Bounds2i ScreenBound(Point2i(0, 0), Point2i(WIDTH, HEIGHT));
    std::shared_ptr<Integrator> integrator = std::make_shared<WhittedIntegrator>(5, camera, sampler, ScreenBound, m_pFramebuffer);
    //SamplerIntegrator* integrator = new SamplerIntegrator(camera, nullptr, ScreenBound, m_pFramebuffer);
    
    //初始化面光源
    emit PrintString((char*)"Init AreaLight");
    
    // 面光源
    std::vector<std::shared_ptr<Light>> lights;
    int nTrianglesAreaLight = 2;
    int vertexIndicesAreaLight[6] = {0, 1, 2, 3, 4, 5};
    int nVerticesAreaLight = 6;
    const float yPos_AreaLight = 0.0;
    Point3f P_AreaLight[6] = { Point3f(-1.4,0.0,1.4), Point3f(-1.4,0.0,-1.4), Point3f(1.4,0.0,1.4),
        Point3f(1.4,0.0,1.4), Point3f(-1.4,0.0,-1.4), Point3f(1.4,0.0,-1.4)};

    Transform tri_Object2World_AreaLight = Translate(Vector3f(0.7f, 5.0f, -2.0f));
    Transform tri_World2Object_AreaLight = Inverse(tri_Object2World_AreaLight);

    std::shared_ptr<TriangleMesh> meshAreaLight = std::make_shared<TriangleMesh>
        (tri_Object2World_AreaLight, nTrianglesAreaLight, vertexIndicesAreaLight, nVerticesAreaLight, P_AreaLight, nullptr, nullptr, nullptr, nullptr);
    std::vector<std::shared_ptr<Shape>> trisAreaLight;
  
    for (int i = 0; i < nTrianglesAreaLight; ++i)
        trisAreaLight.push_back(std::make_shared<Triangle>(&tri_Object2World_AreaLight, &tri_World2Object_AreaLight, false, meshAreaLight, i));
    //
    for (int i = 0; i < nTrianglesAreaLight; ++i)
    {
        std::shared_ptr<AreaLight> area =
            std::make_shared<DiffuseAreaLight>(tri_Object2World_AreaLight, Spectrum(25.0f), 5, trisAreaLight[i], false);
        lights.push_back(area);
        prims.push_back(std::make_shared<GeometricPrimitive>(trisAreaLight[i], floorMaterial, area));
    }
    
    //构造环境光源
    Transform SkyBoxToWorld;
    Point3f SkyBoxCenter(0.f, 0.f, 0.f);
    Float SkyBoxRadius = 10.0f;
    std::shared_ptr<Light> skyBoxLight = std::make_shared<SkyBoxLight>(SkyBoxToWorld, SkyBoxCenter, SkyBoxRadius, "1", 1);
    lights.push_back(skyBoxLight);
    
    emit PrintString((char*)"Init worldScene");
    std::unique_ptr<Scene> worldScene = std::make_unique<Scene>(std::make_shared<BVHAccel>(prims, 1), lights);
    
    
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
	}
	
}
