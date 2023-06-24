
// lights/spot.cpp*
#include "SpotLight.h"
#include "core/Sampling.h"
#include "core/Reflection.h"

namespace pbr
{

// SpotLight Method Definitions
SpotLight::SpotLight(const Transform &LightToWorld,
                     const MediumInterface &mediumInterface, const Spectrum &I,
                     Float totalWidth, Float falloffStart)
    : Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
      pLight(LightToWorld(Point3f(0, 0, 0))),
      I(I),
      cosTotalWidth(std::cos(Radians(totalWidth))),
      cosFalloffStart(std::cos(Radians(falloffStart))) {}

Spectrum SpotLight::Sample_Li(const Interaction &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf,
                              VisibilityTester *vis) const
{
    //ProfilePhase _(Prof::LightSample);
    *wi = Normalize(pLight - ref.p);
    *pdf = 1.f;
    *vis = VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
    return I * Falloff(-*wi) / DistanceSquared(pLight, ref.p);  //计算最终的光强
}

Float SpotLight::Falloff(const Vector3f &w) const
{
    Vector3f wl = Normalize(WorldToLight(w));
    Float cosTheta = wl.z;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta >= cosFalloffStart) return 1;
    // Compute falloff inside spotlight cone
    Float delta = (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);  //当前角度对比整个变化角度的比例
    return (delta * delta) * (delta * delta);
}

Spectrum SpotLight::Power() const
{
    return I * 2 * Pi * (1 - .5f * (cosFalloffStart + cosTotalWidth));
}

Float SpotLight::Pdf_Li(const Interaction &, const Vector3f &) const
{
    return 0.f;
}

Spectrum SpotLight::Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                              Ray *ray, Normal3f *nLight, Float *pdfPos,
                              Float *pdfDir) const
{
    //ProfilePhase _(Prof::LightSample);
    Vector3f w = UniformSampleCone(u1, cosTotalWidth);
    *ray = Ray(pLight, LightToWorld(w), Infinity, time, mediumInterface.inside);
    *nLight = (Normal3f)ray->d;
    *pdfPos = 1;
    *pdfDir = UniformConePdf(cosTotalWidth);
    return I * Falloff(ray->d);
}

void SpotLight::Pdf_Le(const Ray &ray, const Normal3f &, Float *pdfPos, Float *pdfDir) const
{
    //ProfilePhase _(Prof::LightPdf);
    *pdfPos = 0;
    *pdfDir = (CosTheta(WorldToLight(ray.d)) >= cosTotalWidth)
                  ? UniformConePdf(cosTotalWidth)
                  : 0;
}

}  // namespace pbr
