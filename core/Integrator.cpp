
// core/integrator.cpp*
#include "Integrator.h"
#include "Scene.h"
#include "Interaction.h"
#include "Sampling.h"
#include "Sampler.h"
#include "Integrator.h"
#include "Camera.h"
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

            std::unique_ptr<Sampler> pixel_sampler = sampler->Clone(offset);
            Point2i pixel(i, j);
            pixel_sampler->StartPixel(pixel);

            CameraSample cs;
            cs = pixel_sampler->GetCameraSample(pixel);
            Ray r;
            camera->GenerateRay(cs, &r);

            SurfaceInteraction isect;
            Spectrum colObj(0.0f); colObj[0] = 1.0f; colObj[1] = 1.0f;
            if (scene.Intersect(r, &isect)) {
                Vector3f LightNorm = Normalize(Light - isect.p);
                Vector3f viewInv = -r.d;
                //半程向量
                Vector3f H = Normalize(viewInv + LightNorm);
                //高光
                float Ls = Dot(H, isect.n); Ls = (Ls > 0.0f) ? Ls : 0.0f;
                Ls = pow(Ls, 32);
                //漫反射
                float Ld = Dot(LightNorm, isect.n); Ld = (Ld > 0.0f) ? Ld : 0.0f;
                float Li = (0.2 + 0.2 * Ld + 0.6 * Ls);
                colObj = std::abs(Li) * colObj; //防止为负
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
