
// integrators/path.cpp*
#include "PathIntegrator.h"
//#include "bssrdf.h"
#include "core/Camera.h"
//#include "film.h"
#include "core/Interaction.h"
#include "core/Scene.h"
#include "core/Reflection.h"
#include "core/Sampler.h"
#include "core/Light.h"

namespace pbr
{

static int zeroRadiancePaths = 0;
static int totalPaths = 0;
static int pathLength = 0;

// PathIntegrator Method Definitions
PathIntegrator::PathIntegrator(int maxDepth,
                               std::shared_ptr<const Camera> camera,
                               std::shared_ptr<Sampler> sampler,
                               const Bounds2i &pixelBounds, FrameBuffer * pFrameBuffer, Float rrThreshold,
                               const std::string &lightSampleStrategy)
    : SamplerIntegrator(camera, sampler, pixelBounds, pFrameBuffer),
      maxDepth(maxDepth),
      rrThreshold(rrThreshold),
      lightSampleStrategy(lightSampleStrategy) {}

void PathIntegrator::Preprocess(const Scene &scene, Sampler &sampler)
{
    lightDistribution = CreateLightSampleDistribution(lightSampleStrategy, scene);
}

/**
 具体来说，路径追踪包含以下主要步骤：

 1、初始化变量和参数：函数开始时，初始化路径的辐射能量L为0，并且将路径的权重beta初始化为1。还定义了一些用于跟踪光线经过折射边界时的辐射能量缩放的变量。

 2、进入循环：路径追踪算法是一个迭代的过程，每次迭代都代表光线的一次反射或折射。循环不断地执行下面的步骤，直到满足终止条件退出循环。

 3、判断光线是否与场景中的物体相交：调用scene.Intersect()函数判断当前光线是否与场景中的物体相交，并将交点信息存储在SurfaceInteraction对象isect中。

 4、计算路径的直接光照贡献：如果是第一次反射或者上一次是镜面反射（即specularBounce为true），则计算在当前交点处的直接光照贡献。如果找到了交点，则将该交点处的自发辐射Le加到路径的辐射能量L中；如果没有找到交点，则遍历场景中的所有无限大光源，计算该光源对当前路径的贡献并累加到路径的辐射能量L中。

 5、判断路径是否结束：如果没有找到交点（光线逃离了场景）或者迭代次数超过了设定的最大深度，就终止路径追踪，跳出循环。

 6、计算散射函数和跳过介质边界：根据当前交点的材质属性，调用isect.ComputeScatteringFunctions()计算交点处的散射函数（BSDF），并在介质边界上跳过光线。

 7、采样直接光照贡献：根据散射函数的类型，判断是否需要从光源中采样直接光照的贡献，通过UniformSampleOneLight()函数来实现。如果可以采样到有效的光照贡献，则将该贡献乘以权重beta，并累加到路径的辐射能量L中。

 8、采样散射函数得到新的光线方向：根据散射函数的类型，在当前交点处采样反射或折射光线的方向，并计算采样得到的光线方向对应的散射函数值f，以及采样得到的方向的概率密度pdf。

 9、更新路径的辐射能量和权重：根据采样得到的散射函数值f，更新路径的辐射能量L和权重beta。更新后的beta等于原先的beta乘以散射函数值f与新光线方向与法线之间的夹角余弦绝对值除以采样得到的方向的概率密度pdf。

 10、终止路径的追踪：如果路径的权重beta经过一定的迭代次数后的最大分量小于某个阈值（rrThreshold），并且迭代次数超过了一定的阈值（3），就有一定概率直接终止路径的追踪，这是通过俄罗斯转盘法则实现的。

 11、返回路径的辐射能量：迭代结束后，返回路径的累积辐射能量L。
 */

Spectrum PathIntegrator::Li(const RayDifferential &r, const Scene &scene,
                            Sampler &sampler, MemoryArena &arena,
                            int depth) const
{
    Spectrum L(0.f), beta(1.f);
    Ray ray(r);
    bool specularBounce = false;
    int bounces;
    // Added after book publication: etaScale tracks the accumulated effect
    // of radiance scaling due to rays passing through refractive
    // boundaries (see the derivation on p. 527 of the third edition). We
    // track this value in order to remove it from beta when we apply
    // Russian roulette; this is worthwhile, since it lets us sometimes
    // avoid terminating refracted rays that are about to be refracted back
    // out of a medium and thus have their beta value increased.
    
    /**
     这段代码是在书籍出版之后添加的内容。其中的"etaScale"变量用于追踪光线穿过折射界面时辐射能量缩放的累积效果。这个累积效果是通过路径
     踪算法的公式推导（可参考第三版书中的第527页）得到的。我们需要跟踪这个值，以便在应用俄罗斯转盘法则（Russian
     roulette）时，从beta值中移除它。这样做是有意义的，因为它使我们有时可以避免终止即将从介质中折射回来的折射光线，从而增加其beta值。
     
     简而言之，当光线从一个介质进入另一个介质时，会发生折射，并且光线的辐射能量会发生缩放。这个缩放因子通过etaScale进行累积跟踪。在使
     俄罗斯转盘法则时，我们需要从beta值（路径的权重）中移除这个累积效果，以避免过早地终止那些即将从介质中折射回来的光线。

     这个补充内容的目的是准确地模拟折射和光线传播过程中的辐射能量变化，以提高路径追踪算法的真实感和效果。
     */
    Float etaScale = 1;

    for (bounces = 0; ; ++bounces)
    {
        // Find next path vertex and accumulate contribution
//        VLOG(2) << "Path tracer bounce " << bounces << ", current L = " << L
//                << ", beta = " << beta;

        //计算光线与场景的交点
        SurfaceInteraction isect;
        bool foundIntersection = scene.Intersect(ray, &isect);

        // Possibly add emitted light at intersection
        if (bounces == 0 || specularBounce) {
            // Add emitted light at path vertex or from the environment
            if (foundIntersection) {
                L += beta * isect.Le(-ray.d);
                //VLOG(2) << "Added Le -> L = " << L;
            } else {
                for (const auto &light : scene.infiniteLights)
                    L += beta * light->Le(ray);
                //VLOG(2) << "Added infinite area lights -> L = " << L;
            }
        }

        // 若光线逃逸当前场景或者达到最大递归深度，则结束路径追踪
        if (!foundIntersection || bounces >= maxDepth)
        {
            break;
        }

        // 计算散射函数并跳过介质边界
        isect.ComputeScatteringFunctions(ray, arena, true);
        if (!isect.bsdf) {
            //VLOG(2) << "Skipping intersection due to null bsdf";
            ray = isect.SpawnRay(ray.d);
            bounces--;
            continue;
        }

        const Distribution1D *distrib = lightDistribution->Lookup(isect.p);

        // 采样直接光照
        // (But skip this for perfectly specular BSDFs.)
        if (isect.bsdf->NumComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0)
        {
            ++totalPaths;
            Spectrum Ld = beta * UniformSampleOneLight(isect, scene, arena,
                                                       sampler, false, distrib);
            //VLOG(2) << "Sampled direct lighting Ld = " << Ld;
            if (Ld.IsBlack()) ++zeroRadiancePaths;
            CHECK_GE(Ld.y(), 0.f);
            L += Ld;
        }

        // 采样bsdf获得新的光线方向
        Vector3f wo = -ray.d, wi;
        Float pdf;
        BxDFType flags;
        Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf,
                                          BSDF_ALL, &flags);
        //VLOG(2) << "Sampled BSDF, f = " << f << ", pdf = " << pdf;
        if (f.IsBlack() || pdf == 0.f) break;
        beta *= f * AbsDot(wi, isect.shading.n) / pdf;
        //VLOG(2) << "Updated beta = " << beta;
        CHECK_GE(beta.y(), 0.f);
        DCHECK(!std::isinf(beta.y()));
        specularBounce = (flags & BSDF_SPECULAR) != 0;
        if ((flags & BSDF_SPECULAR) && (flags & BSDF_TRANSMISSION)) {
            Float eta = isect.bsdf->eta;
            // Update the term that tracks radiance scaling for refraction
            // depending on whether the ray is entering or leaving the
            // medium.
            etaScale *= (Dot(wo, isect.n) > 0) ? (eta * eta) : 1 / (eta * eta);
        }
        ray = isect.SpawnRay(wi);
        
#if 0

        // Account for subsurface scattering, if applicable
        if (isect.bssrdf && (flags & BSDF_TRANSMISSION))
        {
            // Importance sample the BSSRDF
            SurfaceInteraction pi;
            Spectrum S = isect.bssrdf->Sample_S(
                scene, sampler.Get1D(), sampler.Get2D(), arena, &pi, &pdf);
            DCHECK(!std::isinf(beta.y()));
            if (S.IsBlack() || pdf == 0) break;
            beta *= S / pdf;

            // Account for the direct subsurface scattering component
            L += beta * UniformSampleOneLight(pi, scene, arena, sampler, false,
                                              lightDistribution->Lookup(pi.p));

            // Account for the indirect subsurface scattering component
            Spectrum f = pi.bsdf->Sample_f(pi.wo, &wi, sampler.Get2D(), &pdf,
                                           BSDF_ALL, &flags);
            if (f.IsBlack() || pdf == 0) break;
            beta *= f * AbsDot(wi, pi.shading.n) / pdf;
            DCHECK(!std::isinf(beta.y()));
            specularBounce = (flags & BSDF_SPECULAR) != 0;
            ray = pi.SpawnRay(wi);
        }
        
#endif

        // Possibly terminate the path with Russian roulette.
        // Factor out radiance scaling due to refraction in rrBeta.
        //终止路径的追踪：如果路径的权重beta经过一定的迭代次数后的最大分量小于某个阈值（rrThreshold），并且迭代次数超过了一定的阈
        //值（3），就有一定概率直接终止路径的追踪，这是通过俄罗斯转盘法则实现的。
        Spectrum rrBeta = beta * etaScale;
        if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3) {
            Float q = std::max((Float).05, 1 - rrBeta.MaxComponentValue());
            if (sampler.Get1D() < q) break;
            beta /= 1 - q;
            DCHECK(!std::isinf(beta.y()));
        }
    }
    //ReportValue(pathLength, bounces);
    return L;
}

}  // namespace pbr
