
// lights/distant.cpp*
#include "DistantLight.h"
#include "core/Sampling.h"

namespace pbr
{

DistantLight::DistantLight(const Transform& LightToWorld, const Spectrum& intensity, const Vector3f& direction)
    : Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface()),
      mIntensity(intensity),
      mDirection(Normalize(LightToWorld(direction)))
{
}

Spectrum DistantLight::Sample_Li(const Interaction &ref, const Point2f &u,
                                 Vector3f *wi, Float *pdf,
                                 VisibilityTester *vis) const
{
    *wi = mDirection;
    *pdf = 1;
    Point3f pOutside = ref.p + mDirection * (2 * worldRadius);
    *vis = VisibilityTester(ref, Interaction(pOutside, ref.time, mediumInterface));
    return mIntensity;
}

Spectrum DistantLight::Power() const
{
    return mIntensity * Pi * worldRadius * worldRadius;
}

Float DistantLight::Pdf_Li(const Interaction &, const Vector3f &) const
{
    return 0.f;
}

Spectrum DistantLight::Sample_Le(const Point2f &u1, const Point2f &u2,
                                 Float time, Ray *ray, Normal3f *nLight,
                                 Float *pdfPos, Float *pdfDir) const
{
    // Choose point on disk oriented toward infinite light direction
    Vector3f v1, v2;
    CoordinateSystem(mDirection, &v1, &v2);
    Point2f cd = ConcentricSampleDisk(u1);
    Point3f pDisk = worldCenter + worldRadius * (cd.x * v1 + cd.y * v2);

    // Set ray origin and direction for infinite light ray
    *ray = Ray(pDisk + worldRadius * mDirection, -mDirection, Infinity, time);
    *nLight = (Normal3f)ray->d;
    *pdfPos = 1 / (Pi * worldRadius * worldRadius);
    *pdfDir = 1;
    return mIntensity;
}

void DistantLight::Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos, Float *pdfDir) const
{
    *pdfPos = 1 / (Pi * worldRadius * worldRadius);
    *pdfDir = 0;
}

} // namespace pbr
