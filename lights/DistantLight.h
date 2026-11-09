
#ifndef PBR_LIGHTS_DISTANT_H
#define PBR_LIGHTS_DISTANT_H

// lights/distant.h*
#include "core/Light.h"
#include "core/Shape.h"
#include "core/Scene.h"

namespace pbr
{

// 平行光（无限远光源）
class DistantLight : public Light
{
public:
    DistantLight(const Transform &LightToWorld, const Spectrum &intensity, const Vector3f &direction);

    void Preprocess(const Scene &scene)
    {
        scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
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
    const Spectrum mIntensity;
    const Vector3f mDirection; // 光线方向，单位向量
    Point3f worldCenter;
    Float worldRadius;
};

} // namespace pbr

#endif // PBR_LIGHTS_DISTANT_H
