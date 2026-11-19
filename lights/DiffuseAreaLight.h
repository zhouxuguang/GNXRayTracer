
#ifndef PBR_LIGHTS_DIFFUSE_H
#define PBR_LIGHTS_DIFFUSE_H

// lights/diffuse.h*
#include "core/Light.h"
#include "core/Primitive.h"
#include "core/Spectrum.h"

namespace pbr
{

// Diffuse面积光源
class DiffuseAreaLight : public AreaLight
{
public:
    DiffuseAreaLight(const Transform &LightToWorld, const MediumInterface &mediumInterface,
                     const Spectrum &Le,
                     int nSamples, const std::shared_ptr<Shape> &shape);
    Spectrum L(const Interaction &intr, const Vector3f &w) const;
    Spectrum Power() const;
    Spectrum Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wo,
                       Float *pdf, VisibilityTester *vis) const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;
    Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                       Ray *ray, Normal3f *nLight, Float *pdfPos,
                       Float *pdfDir) const;
    void Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                Float *pdfDir) const;

private:
    const Spectrum mLemit;             // 发光强度
    std::shared_ptr<Shape> mShape;     // 光源形状
    const Float mArea;                 // 光源面积
};

} // namespace pbr

#endif // PBR_LIGHTS_DIFFUSE_H
