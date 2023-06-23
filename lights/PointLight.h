
#ifndef PBR_LIGHTS_POINT_H
#define PBR_LIGHTS_POINT_H

// lights/point.h*
#include "core/Light.h"
#include "core/Shape.h"
#include "core/Spectrum.h"

namespace pbr
{

// PointLight Declarations
class PointLight : public Light
{
public:
    // PointLight Public Methods
    PointLight(const Transform &LightToWorld, const MediumInterface &mediumInterface, const Spectrum &I)
        : Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
          pLight(LightToWorld(Point3f(0, 0, 0))),
          I(I) {}
    Spectrum Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    Spectrum Power() const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;
    Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                Float *pdfDir) const;

private:
    // PointLight Private Data
    const Point3f pLight;
    const Spectrum I;
};

}  // namespace pbr

#endif  // PBR_LIGHTS_POINT_H
