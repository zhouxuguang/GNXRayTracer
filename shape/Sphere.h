#ifndef PBR_SHAPES_SPHERE_H
#define PBR_SHAPES_SPHERE_H

#include "core/Shape.h"

namespace pbr
{

// 球体
class Sphere : public Shape
{
public:
    Sphere(const Transform *ObjectToWorld, const Transform *WorldToObject, Float radius, const Point3f &center = Point3f(0.f, 0.f, 0.f));

    Bounds3f ObjectBound() const override;
    // center / radius 已是世界空间量（加载器烘焙了平移与缩放），ObjectToWorld 只用于
    // 球面参数化，因此世界包围盒直接取自 ObjectBound()，不再做二次变换
    Bounds3f WorldBound() const override;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture = true) const override;
    bool IntersectP(const Ray &ray, bool testAlphaTexture = true) const override;
    Float Area() const override;
    Interaction Sample(const Point2f &u, Float *pdf) const override;

private:
    Float radius;      //半径
    Point3f center;    //球心
};

} // namespace pbr

#endif // PBR_SHAPES_SPHERE_H
