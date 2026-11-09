
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
    // attenuations: (A 二次项, B 一次项, C 常数项)，默认 (1,0,0) 即物理平方反比
    PointLight(const Transform &LightToWorld, const MediumInterface &mediumInterface,
               const Spectrum &intensity,
               const Vector3f &attenuations = Vector3f(1.f, 0.f, 0.f))
        : Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
          mPosition(LightToWorld(Point3f(0, 0, 0))),
          mIntensity(intensity),
          mAttenuations(attenuations)
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
    const Point3f mPosition;       //光源位置
    const Spectrum mIntensity;     //光源强度
    const Vector3f mAttenuations;  //(A 二次项, B 一次项, C 常数项)
};

} // namespace pbr

#endif // PBR_LIGHTS_POINT_H
