#include "RenderThread.h"
#include "DebugText.h"
#include <QTime>

#include "RenderStatus.h"

#include <stdlib.h>
#include <time.h>

#include "shape/Triangle.h"
#include "shape/plyRead.h"
#include "core/Interaction.h"
#include "accelerator/BVHAccel.h"
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
        
    Aggregate *agg = new BVHAccel(prims, 1);
    
    //相机参数
    Vector3f lower_left_corner(-2.0, -2.0, -2.0);
    Vector3f horizontal(4.0, 0.0, 0.0);
    Vector3f vertical(0.0, 4.0, 0.0);
    Point3f origin(0.0, 0.0, -4.0);
    
    
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

				Ray r(origin, (lower_left_corner + u*horizontal + v*vertical) - Vector3f(origin));
				SurfaceInteraction isect;
                
                Float tHit;
                Vector3f colObj;
//                if (tris[0]->Intersect(r, &tHit, &isect) || tris[1]->Intersect(r, &tHit, &isect))
//                {
//                    colObj = Vector3f(1.0 , 0.0 , 0.0);
//                }
                
//                for (int count = 0; count < plyi->nTriangles; ++count)
//                {
//                    if (tris[count]->Intersect(r, &tHit, &isect))
//                    {
//                        colObj = Vector3f(1.0, 0.0, 0.0) ;
//                        break;
//                    }
//                }
                
                if (agg->Intersect(r, &isect))
                {
                    colObj = Vector3f(1.0, 0.0, 0.0);
                }

                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 0, renderCount, colObj.x);
                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 1, renderCount, colObj.y);
                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 2, renderCount, colObj.z);
                m_pFramebuffer->set_uc(i, HEIGHT - j - 1, 3, 255);
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
