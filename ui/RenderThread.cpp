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
#include "samplers/Halton.h"
#include <omp.h>

using namespace pbr;


inline void ClockRandomInit()
{
	srand((unsigned)time(NULL));
}

inline double getClockRandom()
{
	return rand() / (RAND_MAX + 1.0);
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
    
    Transform tri_Object2World , tri_World2Object ;
//    int vertexIndices [6] = { 0,1,2,3,4,5 };
//    int nVertices = 6;
//    Point3f P[6] = {
//    Point3f(-1.0 ,1.0 ,0.0) , Point3f(-1.0, -1.0, 0.0), Point3f(0.0 ,1.0 ,0.0) , Point3f(1.0 ,1.0 ,0.0), Point3f(1.0, -1.0, 0.0), Point3f(0.0 , -1.0 ,0.0)
//    };
//
//    //存入mesh里，该行代码可以从triangle .cpp中找到
//    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(
//    tri_Object2World, nTriangles, vertexIndices, nVertices, P, nullptr, nullptr, nullptr, nullptr);
//    std::vector<std::shared_ptr<Shape>> tris;
    
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
    for (int i = 0; i < nTrianglesFloor ; ++i)
    {
        trisFloor.push_back(std::make_shared<Triangle>(&tri_Object2World2, &tri_World2Object2, false, meshFloor, i));
    }
    
    //将物体填充到基元
    for (int i = 0; i < nTrianglesFloor ; ++i)
    {
        prims.push_back(std::make_shared<GeometricPrimitive>(trisFloor[i]));
    }

    
    tri_Object2World = Translate(Vector3f(0.0, -2.5, 0.0)) * tri_Object2World;
    tri_World2Object = Inverse(tri_Object2World);
    plyInfo *plyi = new plyInfo("/Users/zhouxuguang/work/opensource/pbrt/pdf/模型文件-1/dragon.3d");
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
        prims.push_back(std::make_shared<GeometricPrimitive>(tris[i]));
    }
    
    emit PrintString((char*)"Init worldScene");
    std::unique_ptr<Scene> worldScene = std::make_unique<Scene>(std::make_shared<BVHAccel>(prims, 1));
    
    //相机参数
    Point3f eye(-3.0f, 1.5f, -3.0f), look(0.0, 0.0, 0.0f);
    Vector3f up(0.0f, 1.0f, 0.0f);
    Transform lookat = LookAt(eye, look, up);

    Transform Camera2World = Inverse(lookat);
    std::shared_ptr<const Camera> camera = std::shared_ptr<Camera>(CreatePerspectiveCamera(WIDTH, HEIGHT, Camera2World));
    
    
    
    emit PrintString((char*)"Init Sampler");
    Bounds2i imageBound(Point2i(0, 0), Point2i(WIDTH , HEIGHT));
    std::shared_ptr<HaltonSampler> sampler = std::make_unique<HaltonSampler>(8, imageBound, false);
    //std::shared_ptr<ClockRandSampler> sampler = std::make_unique<ClockRandSampler>(8, imageBound);
    
    Bounds2i ScreenBound(Point2i(0, 0), Point2i(WIDTH, HEIGHT));
    std::shared_ptr<Integrator> integrator = std::make_shared<SamplerIntegrator>(camera, sampler, ScreenBound, m_pFramebuffer);
    //SamplerIntegrator* integrator = new SamplerIntegrator(camera, nullptr, ScreenBound, m_pFramebuffer);
    
    
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
