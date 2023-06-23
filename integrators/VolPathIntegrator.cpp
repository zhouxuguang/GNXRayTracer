
// integrators/volpath.cpp*
#include "VolPathIntegrator.h"
//#include "bssrdf.h"
#include "core/Reflection.h"
#include "core/Camera.h"
//#include "film.h"
#include "core/Interaction.h"
#include "core/Scene.h"

namespace pbr
{

static uint64_t pathLength = 0;
static uint64_t volumeInteractions = 0;
static uint64_t surfaceInteractions = 0;

// VolPathIntegrator Method Definitions
void VolPathIntegrator::Preprocess(const Scene &scene, Sampler &sampler)
{
    lightDistribution = CreateLightSampleDistribution(lightSampleStrategy, scene);
}

Spectrum VolPathIntegrator::Li(const RayDifferential &r, const Scene &scene,
                               Sampler &sampler, MemoryArena &arena,
                               int depth) const
{
    //ProfilePhase p(Prof::SamplerIntegratorLi);
    Spectrum L(0.f), beta(1.f);
    RayDifferential ray(r);
    bool specularBounce = false;
    int bounces;
    // Added after book publication: etaScale tracks the accumulated effect
    // of radiance scaling due to rays passing through refractive
    // boundaries (see the derivation on p. 527 of the third edition). We
    // track this value in order to remove it from beta when we apply
    // Russian roulette; this is worthwhile, since it lets us sometimes
    // avoid terminating refracted rays that are about to be refracted back
    // out of a medium and thus have their beta value increased.
    Float etaScale = 1;

    for (bounces = 0; ; ++bounces) {
        // Intersect _ray_ with scene and store intersection in _isect_
        SurfaceInteraction isect;
        bool foundIntersection = scene.Intersect(ray, &isect);

        // Sample the participating medium, if present
        MediumInteraction mi;
        if (ray.medium) beta *= ray.medium->Sample(ray, sampler, arena, &mi);
        if (beta.IsBlack()) break;

        // Handle an interaction with a medium or a surface
        if (mi.IsValid()) {
            // Terminate path if ray escaped or _maxDepth_ was reached
            if (bounces >= maxDepth) break;

            ++volumeInteractions;
            // Handle scattering at point in medium for volumetric path tracer
            const Distribution1D *lightDistrib = lightDistribution->Lookup(mi.p);
            L += beta * UniformSampleOneLight(mi, scene, arena, sampler, true,
                                              lightDistrib);

            Vector3f wo = -ray.d, wi;
            mi.phase->Sample_p(wo, &wi, sampler.Get2D());
            ray = mi.SpawnRay(wi);
            specularBounce = false;
        } else {
            ++surfaceInteractions;
            // Handle scattering at point on surface for volumetric path tracer

            // Possibly add emitted light at intersection
            if (bounces == 0 || specularBounce) {
                // Add emitted light at path vertex or from the environment
                if (foundIntersection)
                    L += beta * isect.Le(-ray.d);
                else
                    for (const auto &light : scene.infiniteLights)
                        L += beta * light->Le(ray);
            }

            // Terminate path if ray escaped or _maxDepth_ was reached
            if (!foundIntersection || bounces >= maxDepth) break;

            // Compute scattering functions and skip over medium boundaries
            isect.ComputeScatteringFunctions(ray, arena, true);
            if (!isect.bsdf) {
                ray = isect.SpawnRay(ray.d);
                bounces--;
                continue;
            }

            // Sample illumination from lights to find attenuated path
            // contribution
            const Distribution1D *lightDistrib =
                lightDistribution->Lookup(isect.p);
            L += beta * UniformSampleOneLight(isect, scene, arena, sampler,
                                              true, lightDistrib);

            // Sample BSDF to get new path direction
            Vector3f wo = -ray.d, wi;
            Float pdf;
            BxDFType flags;
            Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf,
                                              BSDF_ALL, &flags);
            if (f.IsBlack() || pdf == 0.f) break;
            beta *= f * AbsDot(wi, isect.shading.n) / pdf;
            DCHECK(std::isinf(beta.y()) == false);
            specularBounce = (flags & BSDF_SPECULAR) != 0;
            if ((flags & BSDF_SPECULAR) && (flags & BSDF_TRANSMISSION)) {
                Float eta = isect.bsdf->eta;
                // Update the term that tracks radiance scaling for refraction
                // depending on whether the ray is entering or leaving the
                // medium.
                etaScale *=
                    (Dot(wo, isect.n) > 0) ? (eta * eta) : 1 / (eta * eta);
            }
            ray = isect.SpawnRay(wi);

            //次表面反射的内容
            // Account for attenuated subsurface scattering, if applicable
//            if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) {
//                // Importance sample the BSSRDF
//                SurfaceInteraction pi;
//                Spectrum S = isect.bssrdf->Sample_S(
//                    scene, sampler.Get1D(), sampler.Get2D(), arena, &pi, &pdf);
//                DCHECK(std::isinf(beta.y()) == false);
//                if (S.IsBlack() || pdf == 0) break;
//                beta *= S / pdf;
//
//                // Account for the attenuated direct subsurface scattering
//                // component
//                L += beta *
//                     UniformSampleOneLight(pi, scene, arena, sampler, true,
//                                           lightDistribution->Lookup(pi.p));
//
//                // Account for the indirect subsurface scattering component
//                Spectrum f = pi.bsdf->Sample_f(pi.wo, &wi, sampler.Get2D(),
//                                               &pdf, BSDF_ALL, &flags);
//                if (f.IsBlack() || pdf == 0) break;
//                beta *= f * AbsDot(wi, pi.shading.n) / pdf;
//                DCHECK(std::isinf(beta.y()) == false);
//                specularBounce = (flags & BSDF_SPECULAR) != 0;
//                ray = pi.SpawnRay(wi);
//            }
        }

        // Possibly terminate the path with Russian roulette
        // Factor out radiance scaling due to refraction in rrBeta.
        Spectrum rrBeta = beta * etaScale;
        if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3) {
            Float q = std::max((Float).05, 1 - rrBeta.MaxComponentValue());
            if (sampler.Get1D() < q) break;
            beta /= 1 - q;
            DCHECK(std::isinf(beta.y()) == false);
        }
    }
    //ReportValue(pathLength, bounces);
    return L;
}

}  // namespace pbr
