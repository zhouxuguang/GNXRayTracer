
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
#include "MIPMap.h"
#include "ui/FrameBuffer.h"
#include <omp.h>

namespace pbr
{

static int nCameraRays = 0;

// Integrator Method Definitions
Integrator::~Integrator() {}

// Integrator Utility Functions
Spectrum UniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &nLightSamples,
                                bool handleMedia)
{
    //ProfilePhase p(Prof::DirectLighting);
    Spectrum L(0.f);
    for (size_t j = 0; j < scene.lights.size(); ++j) {
        // Accumulate contribution of _j_th light to _L_
        const std::shared_ptr<Light> &light = scene.lights[j];
        int nSamples = nLightSamples[j];
        const Point2f *uLightArray = sampler.Get2DArray(nSamples);
        const Point2f *uScatteringArray = sampler.Get2DArray(nSamples);
        if (!uLightArray || !uScatteringArray) {
            // Use a single sample for illumination from _light_
            Point2f uLight = sampler.Get2D();
            Point2f uScattering = sampler.Get2D();
            L += EstimateDirect(it, uScattering, *light, uLight, scene, sampler,
                                arena, handleMedia);
        } else {
            // Estimate direct lighting using sample arrays
            Spectrum Ld(0.f);
            for (int k = 0; k < nSamples; ++k)
                Ld += EstimateDirect(it, uScatteringArray[k], *light,
                                     uLightArray[k], scene, sampler, arena,
                                     handleMedia);
            L += Ld / nSamples;
        }
    }
    return L;
}

Spectrum UniformSampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia, const Distribution1D *lightDistrib)
{
    //ProfilePhase p(Prof::DirectLighting);
    // Randomly choose a single light to sample, _light_
    int nLights = int(scene.lights.size());
    if (nLights == 0) return Spectrum(0.f);
    int lightNum = 0;
    Float lightPdf;
    if (lightDistrib) {
        lightNum = lightDistrib->SampleDiscrete(sampler.Get1D(), &lightPdf);
        if (lightPdf == 0) return Spectrum(0.f);
    } else {
        lightNum = std::min((int)(sampler.Get1D() * nLights), nLights - 1);
        lightPdf = Float(1) / nLights;
    }
    const std::shared_ptr<Light> &light = scene.lights[lightNum];
    Point2f uLight = sampler.Get2D();
    Point2f uScattering = sampler.Get2D();
    return EstimateDirect(it, uScattering, *light, uLight,
                          scene, sampler, arena, handleMedia) / lightPdf;
}

/**
 这段代码是为了估计直接光照的贡献。函数EstimateDirect接受多个参数，包括交互对象it、散射点uScattering、光源light、光源采样点uLight、场景scene、采样器sampler、内存区域arena、是否处理介质handleMedia和是否考虑镜面反射specular等。

 首先，函数中定义了一系列变量，包括光线方向wi、光源采样pdf值lightPdf、散射pdf值scatteringPdf和能量贡献值Ld等。

 然后，函数使用多重重要性采样对光源进行采样，得到光源的辐射能量Li和光线方向wi。如果光源采样pdf值大于0且光源的辐射能量不为零，则计算出BSDF（双向散射分布函数）或相位函数在光源采样方向上的值。如果交互对象是表面交互类型，则根据光源采样策略评估BSDF的值，并计算出散射pdf值。如果交互对象是介质交互类型，则根据光源采样策略评估相位函数的值，并将其作为BSDF的值。如果BSDF的值不为零，则根据能见度判断光源是否可见，并根据光源是否是delta光源来计算贡献能量。最后，将光源的贡献能量加到总的贡献能量Ld中。

 接着，函数使用多重重要性采样对BSDF进行采样。如果光源不是delta光源，则根据表面交互类型评估散射方向并计算散射pdf值，或者根据介质交互类型评估散射方向并计算散射pdf值。如果散射方向不为零且散射pdf值大于0，则对光源的贡献能量进行考虑。如果采样不是镜面反射，则计算光源的采样pdf值，并根据多重重要性采样原理计算权重。然后，通过光线与场景求交，计算透射率，判断是否找到了表面交互，如果找到了则判断表面交互所在的primitive是否是当前光源的Area Light，如果是，则计算光源的辐射能量，如果没有找到表面交互，则计算光源的辐射能量。最后，将光源的贡献能量加到总的贡献能量Ld中。

 最后，函数返回总的贡献能量Ld。
 */

Spectrum EstimateDirect(const Interaction &it, const Point2f &uScattering,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        MemoryArena &arena, bool handleMedia, bool specular)
{
    BxDFType bsdfFlags =
        specular ? BSDF_ALL : BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
    Spectrum Ld(0.f);
    // Sample light source with multiple importance sampling
    Vector3f wi;
    Float lightPdf = 0, scatteringPdf = 0;
    VisibilityTester visibility;
    
    //采样光
    Spectrum Li = light.Sample_Li(it, uLight, &wi, &lightPdf, &visibility);
//    VLOG(2) << "EstimateDirect uLight:" << uLight << " -> Li: " << Li << ", wi: "
//            << wi << ", pdf: " << lightPdf;
    
    //如果采样光的Pdf>0且光源不是黑的(即光有发出能量)，就可以采样光源
    if (lightPdf > 0 && !Li.IsBlack()) {
        // Compute BSDF or phase function's value for light sample
        Spectrum f;
        if (it.IsSurfaceInteraction()) {
            // Evaluate BSDF for light sampling strategy
            const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
            f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.shading.n);
            scatteringPdf = isect.bsdf->Pdf(isect.wo, wi, bsdfFlags);
            //VLOG(2) << "  surf f*dot :" << f << ", scatteringPdf: " << scatteringPdf;
        }
        
        //参与介质
        else {
            // Evaluate phase function for light sampling strategy
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float p = mi.phase->p(mi.wo, wi);
            f = Spectrum(p);
            scatteringPdf = p;
            //VLOG(2) << "  medium p: " << p;
        }
        if (!f.IsBlack()) {
            // Compute effect of visibility for light source sample
            if (handleMedia) {
                Li *= visibility.Tr(scene, sampler);
                //VLOG(2) << "  after Tr, Li: " << Li;
            } else {
              if (!visibility.Unoccluded(scene)) {
                //VLOG(2) << "  shadow ray blocked";
                Li = Spectrum(0.f);
              } else
                //VLOG(2) << "  shadow ray unoccluded";
                  printf("  shadow ray unoccluded");
            }

            // Add light's contribution to reflected radiance
            if (!Li.IsBlack()) {
                if (IsDeltaLight(light.flags))
                    Ld += f * Li / lightPdf;
                else {
                    Float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
                    Ld += f * Li * weight / lightPdf;
                }
            }
        }
    }

    // Sample BSDF with multiple importance sampling， 如果当前光源不是Delta类型的光源(不是点光源、方向光源)，就可以采样BSDF
    if (!IsDeltaLight(light.flags)) {
        Spectrum f;
        bool sampledSpecular = false;
        if (it.IsSurfaceInteraction()) {
            // Sample scattered direction for surface interactions
            BxDFType sampledType;
            const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
            f = isect.bsdf->Sample_f(isect.wo, &wi, uScattering, &scatteringPdf,
                                     bsdfFlags, &sampledType);
            f *= AbsDot(wi, isect.shading.n);
            sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
        }
        
        //参与介质
        else {
            // Sample scattered direction for medium interactions
            const MediumInteraction &mi = (const MediumInteraction &)it;
            Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
            f = Spectrum(p);
            scatteringPdf = p;
        }
//        VLOG(2) << "  BSDF / phase sampling f: " << f << ", scatteringPdf: " <<
//            scatteringPdf;
        if (!f.IsBlack() && scatteringPdf > 0) {
            // Account for light contributions along sampled direction _wi_
            Float weight = 1;
            if (!sampledSpecular) {
                lightPdf = light.Pdf_Li(it, wi);
                if (lightPdf == 0) return Ld;
                weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
            }

            // Find intersection and compute transmittance
            SurfaceInteraction lightIsect;
            Ray ray = it.SpawnRay(wi);
            Spectrum Tr(1.f);
            bool foundSurfaceInteraction =
                handleMedia ? scene.IntersectTr(ray, sampler, &lightIsect, &Tr)
                            : scene.Intersect(ray, &lightIsect);

            // Add light contribution from material sampling
            Spectrum Li(0.f);
            if (foundSurfaceInteraction) {
                if (lightIsect.primitive->GetAreaLight() == &light)
                    Li = lightIsect.Le(-wi);
            } else
                Li = light.Le(ray);
            if (!Li.IsBlack()) Ld += f * Li * Tr * weight / scatteringPdf;
        }
    }
    return Ld;
}

std::unique_ptr<Distribution1D> ComputeLightPowerDistribution(const Scene &scene)
{
    if (scene.lights.empty()) return nullptr;
    std::vector<Float> lightPower;
    for (const auto &light : scene.lights)
        lightPower.push_back(light->Power().y());
    return std::unique_ptr<Distribution1D>(
        new Distribution1D(&lightPower[0], lightPower.size()));
}

//#define TEST_MIPMAP

// SamplerIntegrator Method Definitions
void SamplerIntegrator::Render(const Scene &scene, double &timeConsume)
{
    Preprocess(scene, *sampler);
    double start = omp_get_wtime(); //渲染开始时间

    m_FrameBuffer->renderCountIncrease();
    
    std::shared_ptr<MIPMap<Spectrum>> mp = nullptr;
    
#ifdef TEST_MIPMAP
    
    int mW = 400, mH = 235;
    Spectrum *mpdata = new Spectrum[mW * mH];
    for (int j = 0; j < mH; j ++)
    {
        for (int i = 0; i <mW; i++)
        {
            int offset = i + j * mW;
            Spectrum s;
            s[0] = i / (Float)pixelBounds.pMax.x;
            s[1] = j / (Float)pixelBounds.pMax.y;
            s[2] = 0.4f;
            mpdata[offset] = s;
        }
    }
    
    mp = std::make_shared<MIPMap<Spectrum>>(Point2i(mW, mH), mpdata, true);
    delete [] mpdata;
    
#endif

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
//                Ray ray;
//                camera->GenerateRay(cs, &r);
                
                RayDifferential ray;
                float rayWeight = camera->GenerateRayDifferential(cs, &ray);
                ray.ScaleDifferentials(1 / std::sqrt((Float)pixel_sampler->samplesPerPixel));

#ifndef TEST_MIPMAP
                colObj += Li(ray, scene, *pixel_sampler, arena, 0);
                
#else
                // 这里可以测试一些东西
#endif
            } while (pixel_sampler->StartNextSample());
            
            colObj = colObj / pixel_sampler->samplesPerPixel;
            
            if (mp)
            {
                if (i < mp->Width() - 1 && j < mp->Height() - 1)
                {
                    colObj = mp->Lookup(Point2f((i + 1) / (Float)pixelBounds.pMax.x, (j + 1) / (Float)pixelBounds.pMax.y), 0.0f);
                }
                else
                {
                    colObj = Spectrum(0.0f);
                }
            }

            m_FrameBuffer->update_f_u_c(i, j, 0, colObj[0]);
            m_FrameBuffer->update_f_u_c(i, j, 1, colObj[1]);
            m_FrameBuffer->update_f_u_c(i, j, 2, colObj[2]);
            m_FrameBuffer->set_uc(i, j, 3, 255);
        }
    }
    
    //m_FrameBuffer->saveToFile("/Users/zhouxuguang/work/source/GNXRayTracer/test.png");

    // 结束时间
    double end = omp_get_wtime();
    timeConsume = end - start;
}

Spectrum SamplerIntegrator::SpecularReflect(
    const RayDifferential &ray, const SurfaceInteraction &isect,
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
        RayDifferential rd = isect.SpawnRay(wi);
        if (ray.hasDifferentials) {
            rd.hasDifferentials = true;
            rd.rxOrigin = isect.p + isect.dpdx;
            rd.ryOrigin = isect.p + isect.dpdy;
            // Compute differential reflected directions
            Normal3f dndx = isect.shading.dndu * isect.dudx +
                            isect.shading.dndv * isect.dvdx;
            Normal3f dndy = isect.shading.dndu * isect.dudy +
                            isect.shading.dndv * isect.dvdy;
            Vector3f dwodx = -ray.rxDirection - wo,
                     dwody = -ray.ryDirection - wo;
            Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
            Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);
            rd.rxDirection =
                wi - dwodx + 2.f * Vector3f(Dot(wo, ns) * dndx + dDNdx * ns);
            rd.ryDirection =
                wi - dwody + 2.f * Vector3f(Dot(wo, ns) * dndy + dDNdy * ns);
        }
        return f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) / pdf;
    }
    else
        return Spectrum(0.f);
}

Spectrum SamplerIntegrator::SpecularTransmit(const RayDifferential &ray,
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
        RayDifferential rd = isect.SpawnRay(wi);
        if (ray.hasDifferentials)
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
        L = f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) / pdf;
    }
    return L;
}


}  // namespace pbr
