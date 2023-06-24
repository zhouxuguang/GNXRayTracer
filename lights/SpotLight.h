
#ifndef PBR_LIGHTS_SPOT_H
#define PBR_LIGHTS_SPOT_H

// lights/spot.h*
#include "core/Light.h"
#include "core/Shape.h"

namespace pbr
{

// SpotLight Declarations
class SpotLight : public Light
{
public:
    // SpotLight Public Methods
    SpotLight(const Transform &LightToWorld, const MediumInterface &m,
              const Spectrum &I, Float totalWidth, Float falloffStart);
    Spectrum Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const;
    Float Falloff(const Vector3f &w) const;
    Spectrum Power() const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;
    Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                Float *pdfDir) const;

private:
    // SpotLight Private Data
    const Point3f pLight;   //光源位置
    const Spectrum I;
    const Float cosTotalWidth, cosFalloffStart;  //分别是光亮度为0的角度，以及开始衰减的角度
};

}  // namespace pbr

#endif  // PBR_LIGHTS_SPOT_H
