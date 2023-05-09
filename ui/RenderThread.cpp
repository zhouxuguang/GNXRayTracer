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
	emit PrintString("Prepared to Render");

	ClockRandomInit();

	int WIDTH = 500;
	int HEIGHT = 500;

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
    
    tri_Object2World = Translate(Vector3f(0.0, -2.5, 0.0)) * tri_Object2World;
    tri_World2Object = Inverse(tri_Object2World);
    plyInfo *plyi = new plyInfo("/Users/zhouxuguang/work/opensource/pbrt/pdf/模型文件-1/dragon.3d");
    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(tri_Object2World, plyi->nTriangles, plyi->vertexIndices, plyi->nVertices, plyi->vertexArray, nullptr, nullptr, nullptr, nullptr);
    std::vector<std::shared_ptr<Shape>> tris;
    std::vector<std::shared_ptr<Primitive>> prims;
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
    
    std::unique_ptr<Scene> worldScene = std::make_unique<Scene>(std::make_shared<BVHAccel>(prims, 1));
    
    //相机参数
    Camera* cam = nullptr;
    Point3f eye(-3.0f, 1.5f, -3.0f), look(0.0, 0.0, 0.0f);
    Vector3f up(0.0f, 1.0f, 0.0f);
    Transform lookat = LookAt(eye, look, up);

    Transform Camera2World = Inverse(lookat);
    cam = CreatePerspectiveCamera(WIDTH, HEIGHT, Camera2World);
    
    Bounds2i imageBound(Point2i(0, 0), Point2i(WIDTH , HEIGHT));
    std::shared_ptr<HaltonSampler> hns = std::make_unique<HaltonSampler>(8, imageBound, false);
    
    
	int renderCount = 0;
	while (renderFlag) {
        QElapsedTimer t;
		t.start();

		//emit PrintString("Rendering");
		renderCount++;
        
        double start = omp_get_wtime();

        #pragma omp parallel for
		for (int i = 0; i < WIDTH; i++) {
			for(int j = 0; j < HEIGHT; j++) {

				float u = float(i + getClockRandom()) / float(WIDTH);
				float v = float(j + getClockRandom()) / float(HEIGHT);
				int offset = (WIDTH * j + i);
                
                std::unique_ptr<Sampler> pixel_sampler = hns->Clone(offset);
                Point2i pixel(i, j);
                pixel_sampler->StartPixel(pixel); //开始准备随机数
                
                Spectrum colObj(0.0f); //colObj[0] = 1.0f; colObj[1] = 1.0f;
                
                do
                {
                    CameraSample cs ;
                    cs= pixel_sampler->GetCameraSample(pixel); //随机数初始化相机采样点
                    
                    Ray r;
                    cam->GenerateRay(cs, &r);
                    
                    SurfaceInteraction isect;
                    
                    Float tHit;
                    
                    Vector3f Light(1.0, 1.0, 1.0);
                    Light = Normalize(Light);
                    
                    if (worldScene->Intersect(r, &isect))
                    {
                        //colObj = Vector3f(1.0, 0.0, 0.0);
                        Float Li = Dot(Light, isect.n);
                        colObj[1] += std::abs(Li);   //取绝对值，防止出现负值
                    }
                    
                } while (pixel_sampler->StartNextSample());

                colObj[1] = colObj [1] / (float)pixel_sampler->samplesPerPixel;

//                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 0, renderCount, colObj[0]);
//                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 1, renderCount, colObj[1]);
//                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 2, renderCount, colObj[2]);
//                m_pFramebuffer->set_uc(i, HEIGHT - j - 1, 3, 255);
                
                m_pFramebuffer->update_f_u_c(i, j, 0, renderCount, colObj[0]);
                m_pFramebuffer->update_f_u_c(i, j, 1, renderCount, colObj[1]);
                m_pFramebuffer->update_f_u_c(i, j, 2, renderCount, colObj[2]);
                m_pFramebuffer->set_uc(i, j, 3, 255);
			}
		}

        double end = omp_get_wtime();
        double frameTime = end - start;
        g_RenderStatus.setDataChanged("Performance", "One Frame Time", QString::number(frameTime), "");
        g_RenderStatus.setDataChanged("Performance", "Frame pre second", QString::number(1.0f / (float)frameTime), "");

		emit PaintBuffer(m_pFramebuffer->getUCbuffer(), WIDTH, HEIGHT, 4);
			
		while (t.elapsed() < 1);
	}
	
}
