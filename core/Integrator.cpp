
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

std::unique_ptr<Distribution1D> ComputeLightPowerDistribution(const Scene &scene)
{
    if (scene.lights.empty()) return nullptr;
    std::vector<Float> lightPower;
    for (const auto &light : scene.lights)
        lightPower.push_back(light->Power().y());
    return std::unique_ptr<Distribution1D>(
        new Distribution1D(&lightPower[0], lightPower.size()));
}

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

Spectrum SamplerIntegrator::SpecularTransmit(const Ray &ray,
                          const SurfaceInteraction &isect,
                          const Scene &scene, Sampler &sampler,
                          MemoryArena &arena, int depth) const
{
    Vector3f wo = isect.wo, wi;
    Float pdf;
    const Point3f &p = isect.p;
    const BSDF &bsdf = *isect.bsdf;
    Spectrum f = bsdf.Sample_f(wo, &wi, sampler.Get2D(), &pdf,
                               BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR));
    Spectrum L = Spectrum(0.f);
    Normal3f ns = isect.shading.n;
    if (pdf > 0.f && !f.IsBlack() && AbsDot(wi, ns) != 0.f) {
        // Compute ray differential _rd_ for specular transmission
        Ray rd = isect.SpawnRay(wi);
        //if (ray.hasDifferentials)
#if 0
        {
            rd.hasDifferentials = true;
            rd.rxOrigin = p + isect.dpdx;
            rd.ryOrigin = p + isect.dpdy;

            Normal3f dndx = isect.shading.dndu * isect.dudx +
                            isect.shading.dndv * isect.dvdx;
            Normal3f dndy = isect.shading.dndu * isect.dudy +
                            isect.shading.dndv * isect.dvdy;

            // The BSDF stores the IOR of the interior of the object being
            // intersected.  Compute the relative IOR by first out by
            // assuming that the ray is entering the object.
            Float eta = 1 / bsdf.eta;
            if (Dot(wo, ns) < 0) {
                // If the ray isn't entering, then we need to invert the
                // relative IOR and negate the normal and its derivatives.
                eta = 1 / eta;
                ns = -ns;
                dndx = -dndx;
                dndy = -dndy;
            }

            /*
              Notes on the derivation:
              - pbrt computes the refracted ray as: \wi = -\eta \omega_o + [ \eta (\wo \cdot \N) - \cos \theta_t ] \N
                It flips the normal to lie in the same hemisphere as \wo, and then \eta is the relative IOR from
                \wo's medium to \wi's medium.
              - If we denote the term in brackets by \mu, then we have: \wi = -\eta \omega_o + \mu \N
              - Now let's take the partial derivative. (We'll use "d" for \partial in the following for brevity.)
                We get: -\eta d\omega_o / dx + \mu dN/dx + d\mu/dx N.
              - We have the values of all of these except for d\mu/dx (using bits from the derivation of specularly
                reflected ray deifferentials).
              - The first term of d\mu/dx is easy: \eta d(\wo \cdot N)/dx. We already have d(\wo \cdot N)/dx.
              - The second term takes a little more work. We have:
                 \cos \theta_i = \sqrt{1 - \eta^2 (1 - (\wo \cdot N)^2)}.
                 Starting from (\wo \cdot N)^2 and reading outward, we have \cos^2 \theta_o, then \sin^2 \theta_o,
                 then \sin^2 \theta_i (via Snell's law), then \cos^2 \theta_i and then \cos \theta_i.
              - Let's take the partial derivative of the sqrt expression. We get:
                1 / 2 * 1 / \cos \theta_i * d/dx (1 - \eta^2 (1 - (\wo \cdot N)^2)).
              - That partial derivatve is equal to:
                d/dx \eta^2 (\wo \cdot N)^2 = 2 \eta^2 (\wo \cdot N) d/dx (\wo \cdot N).
              - Plugging it in, we have d\mu/dx =
                \eta d(\wo \cdot N)/dx - (\eta^2 (\wo \cdot N) d/dx (\wo \cdot N))/(-\wi \cdot N).
             */
            Vector3f dwodx = -ray.rxDirection - wo,
                     dwody = -ray.ryDirection - wo;
            Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
            Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);

            Float mu = eta * Dot(wo, ns) - AbsDot(wi, ns);
            Float dmudx =
                (eta - (eta * eta * Dot(wo, ns)) / AbsDot(wi, ns)) * dDNdx;
            Float dmudy =
                (eta - (eta * eta * Dot(wo, ns)) / AbsDot(wi, ns)) * dDNdy;

            rd.rxDirection =
                wi - eta * dwodx + Vector3f(mu * dndx + dmudx * ns);
            rd.ryDirection =
                wi - eta * dwody + Vector3f(mu * dndy + dmudy * ns);
        }
#endif
        L = f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) / pdf;
    }
    return L;
}


}  // namespace pbr
