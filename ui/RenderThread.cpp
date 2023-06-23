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
#include "materials/PlasticMaterial.h"
#include "materials/MetalMaterial.h"
#include "materials/GlassMaterial.h"
#include "core/Texture.h"
#include "textures/ConstantTexture.h"
#include "textures/ImageTexture.h"
#include "lights/PointLight.h"
#include "lights/DiffuseAreaLight.h"
#include "lights/SkyBoxLight.h"
#include "lights/InfiniteAreaLight.h"
#include "integrators/WhittedIntegrator.h"
#include "integrators/PathIntegrator.h"

#include <omp.h>

using namespace pbr;

#ifdef _WIN32
    #define PATHSPLIT "\\"
#else
    #define PATHSPLIT "/"
#endif

std::string getResourcesDir()
{
    std::string path = __FILE__;
    printf("file path = %s\n", path.c_str());
    std::string::size_type posNX = path.find("GNXRayTracer");
    
    path = path.substr(0, posNX);
    
    path = path + "GNXRayTracer";
    path += PATHSPLIT;
    path += "Resources";
    path += PATHSPLIT;
    return path;
}

void showMemoryInfo(void);

inline std::shared_ptr<Material> getSmileFacePlasticMaterial()
{
    std::unique_ptr<TextureMapping2D> map = std::make_unique<UVMapping2D>(1.f, 1.f, 0.f, 0.f);
    std::string filename = getResourcesDir() + "awesomeface.jpg";
    ImageWrap wrapMode = ImageWrap::Repeat;
    bool trilerp = false;
    float maxAniso = 8.f;
    float scale = 1.f;
    bool gamma = false; //
    std::shared_ptr<Texture<Spectrum>> Kt =
        std::make_shared<ImageTexture<RGBSpectrum, Spectrum>>(std::move(map), filename, trilerp, maxAniso, wrapMode, scale, gamma);

    std::shared_ptr<Texture<float>> plasticRoughness = std::make_shared<ConstantTexture<float>>(0.1f);
    std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);
    return std::make_shared<PlasticMaterial>(Kt, Kt, plasticRoughness, bumpMap, true);
}

inline std::shared_ptr<Material> getPurplePlasticMaterial() 
{
    Spectrum purple; purple[0] = 0.35; purple[1] = 0.12; purple[2] = 0.48;
    std::shared_ptr<Texture<Spectrum>> plasticKd = std::make_shared<ConstantTexture<Spectrum>>(purple);
    std::shared_ptr<Texture<Spectrum>> plasticKr = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(1.f) - purple);
    std::shared_ptr<Texture<float >> plasticRoughness = std::make_shared<ConstantTexture<float >>(0.1f);
    std::shared_ptr<Texture<float >> bumpMap = std::make_shared<ConstantTexture<float >>(0.0f);
    return std::make_shared<PlasticMaterial >(plasticKd, plasticKr, plasticRoughness, bumpMap, true);
}

inline std::shared_ptr<Material> getYelloMetalMaterial() 
{
    Spectrum eta; eta[0] = 0.2f; eta[1] = 0.2f; eta[2] = 0.8f;
    std::shared_ptr<Texture<Spectrum>> etaM = std::make_shared<ConstantTexture<Spectrum>>(eta);
    Spectrum k; k[0] = 0.11f; k[1] = 0.11f; k[2] = 0.11f;
    std::shared_ptr<Texture<Spectrum>> kM = std::make_shared<ConstantTexture<Spectrum>>(k);
    std::shared_ptr<Texture<float>> Roughness = std::make_shared<ConstantTexture<float>>(0.15f);
    std::shared_ptr<Texture<float>> RoughnessU = std::make_shared<ConstantTexture<float>>(0.15f);
    std::shared_ptr<Texture<float>> RoughnessV = std::make_shared<ConstantTexture<float>>(0.15f);
    std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);
    return 	std::make_shared<MetalMaterial>(etaM, kM, Roughness, RoughnessU, RoughnessV, bumpMap, false);
}

inline std::shared_ptr<Material> getWhiteGlassMaterial() 
{
    Spectrum c1; c1[0] = 0.98f; c1[1] = 0.98f; c1[2] = 0.98f;
    std::shared_ptr<Texture<Spectrum>> Kr = std::make_shared<ConstantTexture<Spectrum>>(c1);
    Spectrum c2; c2[0] = 0.98f; c2[1] = 0.98f; c2[2] = 0.98f;
    std::shared_ptr<Texture<Spectrum>> Kt = std::make_shared<ConstantTexture<Spectrum>>(c2);
    std::shared_ptr<Texture<float>> index = std::make_shared<ConstantTexture<float>>(1.5f);
    std::shared_ptr<Texture<float>> RoughnessU = std::make_shared<ConstantTexture<float>>(0.1f);
    std::shared_ptr<Texture<float>> RoughnessV = std::make_shared<ConstantTexture<float>>(0.1f);
    std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);
    return 	std::make_shared<GlassMaterial>(Kr, Kt,
        RoughnessU, RoughnessV, index, bumpMap, false);
}


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

    Transform Camera2World = Inverse(lookat);
    std::shared_ptr<const Camera> camera = std::shared_ptr<Camera>(CreatePerspectiveCamera(WIDTH, HEIGHT, Camera2World));
    
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
//    for (int i = 0; i < nTrianglesFloor; ++i)
//    {
//        prims.push_back(std::make_shared<GeometricPrimitive>(trisFloor[i], whiteWallMaterial, nullptr, MediumInterface()));
//    }

    
    emit PrintString((char*)"Init Mesh...");
    {
        std::shared_ptr<TriangleMesh> mesh;
        std::vector<std::shared_ptr<Shape>> tris;
    
        Transform tri_Object2World, tri_World2Object;

        tri_Object2World = Translate(Vector3f(0.f, -2.9f, 0.f)) * tri_Object2World;
        tri_World2Object = Inverse(tri_Object2World);

        //E:\code\model
    #ifdef _WIN32
        plyInfo plyi("E:\\zxg\\code\\model\\dragon.3d");
    #else
        plyInfo plyi("/Users/zhouxuguang/work/opensource/pbrt/pdf/模型文件-1/dragon.3d");
    #endif // _WIN32
        mesh = std::make_shared<TriangleMesh>(tri_Object2World, plyi.nTriangles, plyi.vertexIndices, plyi.nVertices, plyi.vertexArray, nullptr, nullptr, nullptr, nullptr);
        tris.reserve(plyi.nTriangles);
 
        for (int i = 0; i < plyi.nTriangles; ++i)
            tris.push_back(std::make_shared<Triangle>(&tri_Object2World, &tri_World2Object, false, mesh, i));

        for (int i = 0; i < plyi.nTriangles; ++i)
            prims.push_back(std::make_shared<GeometricPrimitive>(tris[i], dragonMaterial, nullptr, MediumInterface()));
        plyi.Release();
    }
    
    emit PrintString((char*)"Init Cornell Box...");
    {
        //三角形个数
        const int nTrianglesWall = 2 * 5;
        int vertexIndicesWall[nTrianglesWall * 3];
        for (int i = 0; i < nTrianglesWall * 3; i++)
            vertexIndicesWall[i] = i;
        const int nVerticesWall = nTrianglesWall * 3;
        const float length_Wall = 5.0f;
        Point3f P_Wall[nVerticesWall] =
        {
            //底座
            Point3f(0.f,0.f,length_Wall),Point3f(length_Wall,0.f,length_Wall), Point3f(0.f,0.f,0.f),
            Point3f(length_Wall,0.f,length_Wall),Point3f(length_Wall,0.f,0.f),Point3f(0.f,0.f,0.f),
            //天花板
            Point3f(0.f,length_Wall,length_Wall),Point3f(0.f,length_Wall,0.f),Point3f(length_Wall,length_Wall,length_Wall),
            Point3f(length_Wall,length_Wall,length_Wall),Point3f(0.f,length_Wall,0.f),Point3f(length_Wall,length_Wall,0.f),
            //后墙
            Point3f(0.f,0.f,0.f),Point3f(length_Wall,0.f,0.f), Point3f(length_Wall,length_Wall,0.f),
            Point3f(0.f,0.f,0.f), Point3f(length_Wall,length_Wall,0.f),Point3f(0.f,length_Wall,0.f),
            //右墙
            Point3f(0.f,0.f,0.f),Point3f(0.f,length_Wall,length_Wall), Point3f(0.f,0.f,length_Wall),
            Point3f(0.f,0.f,0.f), Point3f(0.f,length_Wall,0.f),Point3f(0.f,length_Wall,length_Wall),
            //左墙
            Point3f(length_Wall,0.f,0.f),Point3f(length_Wall,length_Wall,length_Wall), Point3f(length_Wall,0.f,length_Wall),
            Point3f(length_Wall,0.f,0.f), Point3f(length_Wall,length_Wall,0.f),Point3f(length_Wall,length_Wall,length_Wall)
        };
        Transform tri_ConBox2World = Translate(Vector3f(-0.5*length_Wall,-0.5*length_Wall,-0.5*length_Wall));
        Transform tri_World2ConBox = Inverse(tri_ConBox2World);
        std::shared_ptr<TriangleMesh> meshConBox = std::make_shared<TriangleMesh>
            (tri_ConBox2World, nTrianglesWall, vertexIndicesWall, nVerticesWall, P_Wall, nullptr, nullptr, nullptr, nullptr);
        std::vector<std::shared_ptr<Shape>> trisConBox;
        for (int i = 0; i < nTrianglesWall; ++i)
            trisConBox.push_back(std::make_shared<Triangle>(&tri_ConBox2World, &tri_World2ConBox, false, meshConBox, i));

        //增加三角形到图元
        for (int i = 0; i < nTrianglesWall; ++i)
        {
            if (i == 6 || i == 7)
                prims.push_back(std::make_shared<GeometricPrimitive>(trisConBox[i], redWallMaterial, nullptr, MediumInterface()));
            else if (i == 8 || i == 9)
                prims.push_back(std::make_shared<GeometricPrimitive>(trisConBox[i], blueWallMaterial, nullptr, MediumInterface()));
            else
                prims.push_back(std::make_shared<GeometricPrimitive>(trisConBox[i], whiteWallMaterial, nullptr, MediumInterface()));
        }
    }
    
    
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
    Transform tri_Object2World_AreaLight = Translate(Vector3f(0.0f, 2.45f, 0.0f));
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
            std::make_shared<DiffuseAreaLight>(tri_Object2World_AreaLight, MediumInterface(), Spectrum(5.0f), 5, trisAreaLight[i], false);
        lights.push_back(area);
        prims.push_back(std::make_shared<GeometricPrimitive>(trisAreaLight[i], dragonMaterial, area, MediumInterface()));
    }
    
    //构造环境光源
    Transform SkyBoxToWorld;
    Point3f SkyBoxCenter(0.f, 0.f, 0.f);
    Float SkyBoxRadius = 10.0f;
//    std::shared_ptr<Light> skyBoxLight = std::make_shared<SkyBoxLight>(SkyBoxToWorld, SkyBoxCenter, SkyBoxRadius, "1", 1);
//    lights.push_back(skyBoxLight);
    
    //构造无限远面光源
//    emit PrintString((char*)"Init InfiniteLight...");
//    {
//        Transform InfinityLightToWorld = RotateX(20) * RotateY(-90) * RotateX(-90);
//        Spectrum power(1.0f);
//        std::shared_ptr<Light> infinityLight =
//            std::make_shared<InfiniteAreaLight>(InfinityLightToWorld, power, 10, getResourcesDir() + "MonValley1000.hdr");
//        lights.push_back(infinityLight);
//    }
    
    emit PrintString((char*)"Init worldScene");
    std::unique_ptr<Scene> worldScene = std::make_unique<Scene>(std::make_shared<BVHAccel>(prims, 1), lights);
    
    emit PrintString((char*)"Init Sampler");
    Bounds2i imageBound(Point2i(0, 0), Point2i(WIDTH , HEIGHT));
    //std::shared_ptr<HaltonSampler> sampler = std::make_unique<HaltonSampler>(8, imageBound, false);
    std::shared_ptr<ClockRandSampler> sampler = std::make_unique<ClockRandSampler>(64, imageBound);
    
    Bounds2i ScreenBound(Point2i(0, 0), Point2i(WIDTH, HEIGHT));
    //std::shared_ptr<Integrator> integrator = std::make_shared<WhittedIntegrator>(5, camera, sampler, ScreenBound, m_pFramebuffer);
    std::shared_ptr<Integrator> integrator = std::make_shared<PathIntegrator>(15, camera, sampler, ScreenBound, m_pFramebuffer, 1.f, "spatial");
    
    
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

#ifdef _WIN32


#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib") 
void showMemoryInfo(void)
{

    //  SIZE_T PeakWorkingSetSize; //峰值内存使用
    //  SIZE_T WorkingSetSize; //内存使用
    //  SIZE_T PagefileUsage; //虚拟内存使用
    //  SIZE_T PeakPagefileUsage; //峰值虚拟内存使用

    EmptyWorkingSet(GetCurrentProcess());

    HANDLE handle = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

    g_RenderStatus.setDataChanged("Memory Use", "WorkingSetSize", QString::number(pmc.WorkingSetSize / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakWorkingSetSize", QString::number(pmc.PeakWorkingSetSize / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PagefileUsage", QString::number(pmc.PagefileUsage / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakPagefileUsage", QString::number(pmc.PeakPagefileUsage / 1000.f / 1000.f), "M");

}

#else

#import <mach/mach.h>

void showMemoryInfo(void)
{
    int64_t memoryUsageInByte = 0;
    task_vm_info_data_t vmInfo;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    kern_return_t kernelReturn = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) &vmInfo, &count);
    if(kernelReturn == KERN_SUCCESS)
    {
        memoryUsageInByte = (int64_t) vmInfo.phys_footprint;
        //NSLog(@"Memory in use (in bytes): %lld", memoryUsageInByte);
    }
    else
    {
        //NSLog(@"Error with task_info(): %s", mach_error_string(kernelReturn));
    }
    
    g_RenderStatus.setDataChanged("Memory Use", "WorkingSetSize", QString::number(vmInfo.resident_size / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakWorkingSetSize", QString::number(vmInfo.resident_size_peak / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PagefileUsage", QString::number(vmInfo.phys_footprint / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakPagefileUsage", QString::number(vmInfo.phys_footprint / 1000.f / 1000.f), "M");
    //return memoryUsageInByte;
}

#endif // _WIN32
