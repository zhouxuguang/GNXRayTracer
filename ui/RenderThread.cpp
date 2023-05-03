#include "RenderThread.h"
#include "DebugText.h"
#include <QTime>

#if 0

#include "Core\FeimosRender.h"
#include "Shape\Triangle.h"
#include "Shape\plyRead.h"
#include "Core\primitive.h"
#include "Accelerator\BVHAccel.h"
#include "Core\interaction.h"

#include <omp.h>

#endif

#include "RenderStatus.h"

#include <stdlib.h>
#include <time.h>

#include "shape/Triangle.h"
#include "core/Interaction.h"

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
#if 1
	emit PrintString("Prepared to Render");

	ClockRandomInit();

	int WIDTH = 500;
	int HEIGHT = 500;

    m_pFramebuffer->bufferResize(WIDTH, HEIGHT);
    
    Transform tri_Object2World , tri_World2Object ; int nTriangles = 2;
    int vertexIndices [6] = { 0,1,2,3,4,5 };
    int nVertices = 6;
    Point3f P[6] = {
    Point3f(-1.0 ,1.0 ,0.0) , Point3f(-1.0, -1.0, 0.0), Point3f(0.0 ,1.0 ,0.0) , Point3f(1.0 ,1.0 ,0.0), Point3f(1.0, -1.0, 0.0), Point3f(0.0 , -1.0 ,0.0)
    };
    //存入mesh里，该行代码可以从triangle .cpp中找到
    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(
    tri_Object2World, nTriangles, vertexIndices, nVertices, P, nullptr, nullptr, nullptr, nullptr);
    std::vector<std::shared_ptr<Shape>> tris;
    
    //从mesh转为Shape类型，该代码可以从triangle .cpp文件里找到 tris .reserve(nTriangles);
    for (int i = 0; i < nTriangles; ++i)
    {
        tris.push_back(std::make_shared<Triangle>(&tri_Object2World, &tri_World2Object, false , mesh, i));
    }
    
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

		for (int i = 0; i < WIDTH; i++) {
			for(int j = 0; j < HEIGHT; j++) {

				float u = float(i + getClockRandom()) / float(WIDTH);
				float v = float(j + getClockRandom()) / float(HEIGHT);
				int offset = (WIDTH * j + i);

				Ray r(origin, (lower_left_corner + u*horizontal + v*vertical) - Vector3f(origin));
				SurfaceInteraction isect;
                
                Float tHit;
                Vector3f colObj;
                if (tris[0]->Intersect(r, &tHit, &isect) || tris[1]->Intersect(r, &tHit, &isect))
                {
                    colObj = Vector3f(1.0 , 0.0 , 0.0);
                }

                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 0, renderCount, colObj.x);
                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 1, renderCount, colObj.y);
                m_pFramebuffer->update_f_u_c(i, HEIGHT - j - 1, 2, renderCount, colObj.z);
                m_pFramebuffer->set_uc(i, HEIGHT - j - 1, 3, 255);
			}
		}

		//double end = omp_get_wtime();
		double frameTime = 1;
        g_RenderStatus.setDataChanged("Performance", "One Frame Time", QString::number(frameTime), "");
        g_RenderStatus.setDataChanged("Performance", "Frame pre second", QString::number(1.0f / (float)frameTime), "");

		emit PaintBuffer(m_pFramebuffer->getUCbuffer(), WIDTH, HEIGHT, 4);
			
		while (t.elapsed() < 1);
	}
    
#endif
	
}
