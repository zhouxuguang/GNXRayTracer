
#ifndef PBR_LIGHTS_POINT_H
#define PBR_LIGHTS_POINT_H

// lights/point.h*
#include "core/Light.h"
#include "core/Shape.h"
#include "core/Spectrum.h"

namespace pbr
{

// 点光源
class PointLight : public Light
{
public:
    PointLight(const Transform &LightToWorld, const MediumInterface &mediumInterface, const Spectrum &intensity)
        : Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
          mPosition(LightToWorld(Point3f(0, 0, 0))),
          mIntensity(intensity)
    {
    }
    Spectrum Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    Spectrum Power() const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;
    Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos, Float *pdfDir) const;

private:
    const Point3f mPosition;      //光源位置
    const Spectrum mIntensity;    //光源强度
};

} // namespace pbr

#endif // PBR_LIGHTS_POINT_H
