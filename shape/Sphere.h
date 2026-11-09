#ifndef PBR_SHAPES_SPHERE_H
#define PBR_SHAPES_SPHERE_H

#include "core/Shape.h"

namespace pbr
{

// 球体（世界空间）
// XML 场景加载器把 <Transform> 烘焙进 center 与 radius，
// 因此这里不依赖外部的 ObjectToWorld / WorldToObject 变换。
class Sphere : public Shape
{
public:
    Sphere(Float radius, const Point3f &center = Point3f(0.f, 0.f, 0.f));

    Bounds3f ObjectBound() const override;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture = true) const override;
    bool IntersectP(const Ray &ray, bool testAlphaTexture = true) const override;
    Float Area() const override;
    Interaction Sample(const Point2f &u, Float *pdf) const override;

private:
    Float radius;
    Point3f center;
};

} // namespace pbr

#endif // PBR_SHAPES_SPHERE_H
