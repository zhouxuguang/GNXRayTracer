

#ifndef PBR_CORE_LIGHT_H
#define PBR_CORE_LIGHT_H

// core/light.h*
#include "Memory.h"
#include "Interaction.h"
#include "Transform.h"

namespace pbr
{

// LightFlags Declarations
enum class LightFlags : int {
    DeltaPosition = 1,
    DeltaDirection = 2,
    Area = 4,
    Infinite = 8
};

inline bool IsDeltaLight(int flags)
{
    return flags & (int)LightFlags::DeltaPosition ||
           flags & (int)LightFlags::DeltaDirection;
}

// Light Declarations
class Light
{
public:
    // Light Interface
    virtual ~Light();
    Light(int flags, const Transform &LightToWorld,
          /*const MediumInterface &mediumInterface,*/ int nSamples = 1);
    virtual Spectrum Sample_Li(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const = 0;
    virtual Spectrum Power() const = 0;
    virtual void Preprocess(const Scene &scene) {}
    virtual Spectrum Le(const Ray &r) const;
    virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const = 0;
    virtual Spectrum Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                               Ray *ray, Normal3f *nLight, Float *pdfPos,
                               Float *pdfDir) const = 0;
    virtual void Pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
                        Float *pdfDir) const = 0;

    // Light Public Data
    const int flags;
    const int nSamples;
    //const MediumInterface mediumInterface;

protected:
    // Light Protected Data
    const Transform LightToWorld, WorldToLight;
};

class VisibilityTester
{
public:
    VisibilityTester() {}
    // VisibilityTester Public Methods
    VisibilityTester(const Interaction &p0, const Interaction &p1)
        : p0(p0), p1(p1) {}
    const Interaction &P0() const { return p0; }
    const Interaction &P1() const { return p1; }
    
    //是否没有被遮挡
    bool Unoccluded(const Scene &scene) const;
    Spectrum Tr(const Scene &scene, Sampler &sampler) const;

private:
    Interaction p0, p1;
};

class AreaLight : public Light
{
public:
    // AreaLight Interface
    AreaLight(const Transform &LightToWorld, int nSamples);
    virtual Spectrum L(const Interaction &intr, const Vector3f &w) const = 0;
};

}  // namespace pbr

#endif  // PBR_CORE_LIGHT_H