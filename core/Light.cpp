

// core/light.cpp*
#include "Light.h"
#include "Scene.h"
#include "Sampling.h"
#include "Spectrum.h"

namespace pbr
{

static int numLights = 0;
static int numAreaLights = 0;

// Light Method Definitions
Light::Light(int flags, const Transform &LightToWorld,
             const MediumInterface &mediumInterface, int nSamples)
    : flags(flags),
      nSamples(std::max(1, nSamples)),
      mediumInterface(mediumInterface),
      LightToWorld(LightToWorld),
      WorldToLight(Inverse(LightToWorld)) {
    ++numLights;
}

Light::~Light() {}

bool VisibilityTester::Unoccluded(const Scene &scene) const
{
    return !scene.IntersectP(p0.SpawnRayTo(p1));
}

Spectrum VisibilityTester::Tr(const Scene &scene, Sampler &sampler) const
{
    Ray ray(p0.SpawnRayTo(p1));
    Spectrum Tr(1.f);
    while (true)
    {
        SurfaceInteraction isect;
        bool hitSurface = scene.Intersect(ray, &isect);
        // Handle opaque surface along ray's path
        if (hitSurface && isect.primitive->GetMaterial() != nullptr)
            return Spectrum(0.0f);

        // Update transmittance for current ray segment
        if (ray.medium) Tr *= ray.medium->Tr(ray, sampler);

        // Generate next ray segment or return final transmittance
        if (!hitSurface) break;
        ray = isect.SpawnRayTo(p1);
    }
    return Tr;
}

Spectrum Light::Le(const RayDifferential &ray) const { return Spectrum(0.f); }

AreaLight::AreaLight(const Transform &LightToWorld, const MediumInterface &medium, int nSamples)
    : Light((int)LightFlags::Area, LightToWorld, medium, nSamples)
{
    ++numAreaLights;
}

}  // namespace pbr
