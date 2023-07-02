
#ifndef PBR_LIGHTS_DIFFUSE_H
#define PBR_LIGHTS_DIFFUSE_H

// lights/diffuse.h*
#include "core/Light.h"
#include "core/Primitive.h"
#include "core/Spectrum.h"

namespace pbr
{

// DiffuseAreaLight Declarations
class DiffuseAreaLight : public AreaLight
{
  public:
    // DiffuseAreaLight Public Methods
    DiffuseAreaLight(const Transform &LightToWorld, const MediumInterface &mediumInterface,
                     const Spectrum &Le,
                     int nSamples, const std::shared_ptr<Shape> &shape,
                     bool twoSided = false);
    Spectrum L(const Interaction &intr, const Vector3f &w) const
    {
        bool dotNW = Dot(intr.n, w);
        //dotNW = true;
        return (twoSided || dotNW > 0) ? Lemit : Spectrum(0.f);
    }
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
    // DiffuseAreaLight Protected Data
    const Spectrum Lemit;
    std::shared_ptr<Shape> shape;
    // Added after book publication: by default, DiffuseAreaLights still
    // only emit in the hemimsphere around the surface normal.  However,
    // this behavior can now be overridden to give emission on both sides.
    const bool twoSided;
    const Float area;
};

}  // namespace pbr

#endif  // PBR_LIGHTS_DIFFUSE_H
