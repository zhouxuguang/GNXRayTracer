
// core/integrator.cpp*
#include "Integrator.h"
#include "Scene.h"
#include "Interaction.h"
#include "Sampling.h"
#include "Sampler.h"
#include "Integrator.h"
#include "Camera.h"
#include "Reflection.h"
#include "Light.h"
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
            
            Spectrum colObj(.0f);
            
            do
            {
                CameraSample cs;
                cs = pixel_sampler->GetCameraSample(pixel);
                Ray r;
                camera->GenerateRay(cs, &r);

#if 1
                colObj += Li(r, scene, *pixel_sampler, arena, 0);
                
#else
                SurfaceInteraction isect;
                if (scene.Intersect(r, &isect))
                {
                    VisibilityTester vist;
                    Vector3f wi;
                    float pdf_light; //采样光的Pdf

                    size_t lightCount = scene.lights.size();
                    for (int count = 0; count < lightCount; count++)
                    {
                        Spectrum Li = scene.lights[count]->Sample_Li(isect, pixel_sampler->Get2D(), &wi, &pdf_light, &vist);

                        if (vist.Unoccluded(scene))
                        {
                            //计算散射
                            isect.ComputeScatteringFunctions(r, arena);
                            Vector3f wo = isect.wo;

                            //采样散射光分布函数
                            Spectrum f = isect.bsdf->f(wo, wi);

                            //散射Pdf
                            Float pdf_scattering = isect.bsdf->Pdf(wo, wi);

                            //乘以3.0的意义是为了不让图像过暗
                            colObj += Li * pdf_scattering * f * 3.0f / pdf_light;
                        }
                    }

                    colObj /= lightCount;
                }
                
#endif
            } while (pixel_sampler->StartNextSample());
            
            colObj = colObj / pixel_sampler->samplesPerPixel;
            //printf("colobk = %f, %f, %f\n", colObj[0], colObj[1], colObj[2]);
            
            //colObj[0] = 1.0;

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

Spectrum SamplerIntegrator::SpecularReflect(
    const Ray &ray, const SurfaceInteraction &isect,
    const Scene &scene, Sampler &sampler, MemoryArena &arena, int depth) const
{
    // Compute specular reflection direction _wi_ and BSDF value
    Vector3f wo = isect.wo, wi;
    Float pdf;
    BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
    Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, type);

    // Return contribution of specular reflection
    const Normal3f &ns = isect.shading.n;
    if (pdf > 0.f && !f.IsBlack() && AbsDot(wi, ns) != 0.f)
    {
        // Compute ray differential _rd_ for specular reflection
        Ray rd = isect.SpawnRay(wi);
//        if (ray.hasDifferentials) {
//            rd.hasDifferentials = true;
//            rd.rxOrigin = isect.p + isect.dpdx;
//            rd.ryOrigin = isect.p + isect.dpdy;
//            // Compute differential reflected directions
//            Normal3f dndx = isect.shading.dndu * isect.dudx +
//                            isect.shading.dndv * isect.dvdx;
//            Normal3f dndy = isect.shading.dndu * isect.dudy +
//                            isect.shading.dndv * isect.dvdy;
//            Vector3f dwodx = -ray.rxDirection - wo,
//                     dwody = -ray.ryDirection - wo;
//            Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
//            Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);
//            rd.rxDirection =
//                wi - dwodx + 2.f * Vector3f(Dot(wo, ns) * dndx + dDNdx * ns);
//            rd.ryDirection =
//                wi - dwody + 2.f * Vector3f(Dot(wo, ns) * dndy + dDNdy * ns);
//        }
        return f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) / pdf;
    }
    else
        return Spectrum(0.f);
}


}  // namespace pbr
