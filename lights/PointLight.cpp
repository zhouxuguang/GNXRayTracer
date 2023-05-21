

// lights/point.cpp*
#include "PointLight.h"
#include "core/Scene.h"
#include "core/Sampling.h"
#include "core/Interaction.h"

namespace pbr
{

// PointLight Method Definitions
Spectrum PointLight::Sample_Li(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const
{
    //ProfilePhase _(Prof::LightSample);
    *wi = Normalize(pLight - ref.p);
    *pdf = 1.f;
    *vis = VisibilityTester(ref, Interaction(pLight, ref.time));
    //return I;
    return I / DistanceSquared(pLight, ref.p);
}

Spectrum PointLight::Power() const { return 4 * Pi * I; }

Float PointLight::Pdf_Li(const Interaction &, const Vector3f &) const
{
    return 0;
}

Spectrum PointLight::Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                               Ray *ray, Normal3f *nLight, Float *pdfPos,
                               Float *pdfDir) const
{
    //ProfilePhase _(Prof::LightSample);
    *ray = Ray(pLight, UniformSampleSphere(u1), Infinity, time);
    *nLight = (Normal3f)ray->d;
    *pdfPos = 1;
    *pdfDir = UniformSpherePdf();
    return I;
}

void PointLight::Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos, Float *pdfDir) const
{
    //ProfilePhase _(Prof::LightPdf);
    *pdfPos = 0;
    *pdfDir = UniformSpherePdf();
}

}  // namespace pbrt
