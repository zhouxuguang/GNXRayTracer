

// core/light.cpp*
#include "Light.h"
#include "Scene.h"
#include "Sampling.h"
#include "Spectrum.h"
#include <algorithm>
#include <cmath>

namespace pbr
{

static int numLights = 0;
static int numAreaLights = 0;

Float DistanceAttenuation(Float distanceSquared, const Vector3f &attenuations)
{
    Float distance = std::sqrt(std::max((Float)0, distanceSquared));
    Float denominator = attenuations.z + attenuations.y * distance +
                        attenuations.x * distanceSquared;
    if (denominator <= 0)
        return 0;
    return 1 / denominator;
}

// Light Method Definitions
Light::Light(int flags, const Transform &LightToWorld,
             const MediumInterface &mediumInterface, int nSamples)
    : flags(flags),
      nSamples(std::max(1, nSamples)),
      mediumInterface(mediumInterface),
      LightToWorld(LightToWorld),
      WorldToLight(Inverse(LightToWorld))
{
    ++numLights;
}

Light::~Light()
{
}

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
        if (ray.medium)
            Tr *= ray.medium->Tr(ray, sampler);

        // Generate next ray segment or return final transmittance
        if (!hitSurface)
            break;
        ray = isect.SpawnRayTo(p1);
    }
    return Tr;
}

Spectrum Light::Le(const RayDifferential &ray) const
{
    return Spectrum(0.f);
}

AreaLight::AreaLight(const Transform &LightToWorld, const MediumInterface &medium, int nSamples)
    : Light((int)LightFlags::Area, LightToWorld, medium, nSamples)
{
    ++numAreaLights;
}

} // namespace pbr
