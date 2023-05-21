
// core/integrator.cpp*
#include "Integrator.h"
#include "Scene.h"
#include "Interaction.h"
#include "Sampling.h"
#include "Sampler.h"
#include "Integrator.h"
#include "Camera.h"
#include "Reflection.h"
#include "ui/FrameBuffer.h"
#include <omp.h>

namespace pbr
{

static int nCameraRays = 0;

// Integrator Method Definitions
Integrator::~Integrator() {}

// SamplerIntegrator Method Definitions
void SamplerIntegrator::Render(const Scene &scene, double &timeConsume)
{
    double start = omp_get_wtime(); //渲染开始时间

    m_FrameBuffer->renderCountIncrease();

    //光源位置
    Point3f Light(10.0, 10.0, -10.0);

    #pragma omp parallel for
    for (int i = 0; i < pixelBounds.pMax.x; i++)
    {
        for (int j = 0; j < pixelBounds.pMax.y; j++)
        {

            float u = float(i + getClockRandom()) / float(pixelBounds.pMax.x);
            float v = float(j + getClockRandom()) / float(pixelBounds.pMax.y);
            int offset = (pixelBounds.pMax.x * j + i);
            
            MemoryArena arena;

            std::unique_ptr<Sampler> pixel_sampler = sampler->Clone(offset);
            Point2i pixel(i, j);
            pixel_sampler->StartPixel(pixel);

            CameraSample cs;
            cs = pixel_sampler->GetCameraSample(pixel);
            Ray r;
            camera->GenerateRay(cs, &r);

            SurfaceInteraction isect;
            Spectrum colObj(.0f);
            if (scene.Intersect(r, &isect))
            {
                //计算散射
                isect.ComputeScatteringFunctions(r, arena);
                Vector3f LightNorm = Normalize(Light - isect.p);
                Vector3f wi = LightNorm;
                Vector3f wo = isect.wo;

                Spectrum f = isect.bsdf->f(wo, wi);
                Float pdf = isect.bsdf->Pdf(wo, wi);
                
                //乘以5.0的意义是为了不让图像过暗
                colObj += f * pdf * 5.0f;
            }

            m_FrameBuffer->update_f_u_c(i, j, 0, colObj[0]);
            m_FrameBuffer->update_f_u_c(i, j, 1, colObj[1]);
            m_FrameBuffer->update_f_u_c(i, j, 2, colObj[2]);
            m_FrameBuffer->set_uc(i, j, 3, 255);
        }
    }

    // 结束时间
    double end = omp_get_wtime();
    timeConsume = end - start;
}


}  // namespace pbr
