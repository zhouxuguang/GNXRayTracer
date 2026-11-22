#include "Sphere.h"
#include "core/Interaction.h"
#include <algorithm>
#include <cmath>

namespace pbr
{

Sphere::Sphere(const Transform *ObjectToWorld, const Transform *WorldToObject, Float radius, const Point3f &center)
    : Shape(ObjectToWorld, WorldToObject, false),
      radius(radius),
      center(center)
{
}

Bounds3f Sphere::ObjectBound() const
{
    return Bounds3f(Point3f(center.x - radius, center.y - radius, center.z - radius),
                    Point3f(center.x + radius, center.y + radius, center.z + radius));
}

Bounds3f Sphere::WorldBound() const
{
    return ObjectBound();
}

Float Sphere::Area() const
{
    return 4 * Pi * radius * radius;
}

bool Sphere::Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                       bool testAlphaTexture) const
{
    Vector3f oc = ray.o - center;
    Float a = Dot(ray.d, ray.d);
    if (a <= 0)
        return false;

    Float b = 2 * Dot(oc, ray.d);
    Float c = Dot(oc, oc) - radius * radius;
    Float discriminant = b * b - 4 * a * c;
    if (discriminant <= 0)
        return false;

    Float root = std::sqrt(discriminant);
    Float t = (-b - root) / (2 * a);
    if (t <= 0)
        t = (-b + root) / (2 * a);
    if (t <= 0 || t >= ray.tMax)
        return false;

    Point3f pHit = ray(t);
    Vector3f rel = pHit - center;
    if (tHit)
        *tHit = t;
    ray.tMax = t;

    if (isect)
    {
        // 球面参数化在物体空间进行：<Rotation> 会带动 UV 极点与纹理走向；
        // 几何（center/radius）已是世界空间量，交点位置与外法线仍按世界空间算
        Vector3f relLocal = (*WorldToObject)(rel);
        Vector3f dirLocal = Normalize(relLocal);
        Vector3f tangent = Cross(Vector3f(0, 0, 1), relLocal);
        if (tangent.LengthSquared() < 1e-12f)
            tangent = Cross(Vector3f(0, 1, 0), relLocal);
        Vector3f bitangent = Cross(relLocal, tangent);
        Vector3f normal = Normalize(rel);
        // 参数化切向回到世界空间，供纹理微分使用
        Vector3f dpdu = (*ObjectToWorld)(tangent);
        Vector3f dpdv = (*ObjectToWorld)(bitangent);

        Float phi = std::atan2(dirLocal.y, dirLocal.x);
        if (phi < 0)
            phi += 2 * Pi;
        Float theta = std::acos(Clamp(dirLocal.z, -1.f, 1.f));

        isect->p = pHit;
        isect->time = ray.time;
        // 交点误差界：非零才能驱动 OffsetRayOrigin 把阴影射线推离球面，
        // 否则起点精确落在表面上会自相交，产生阴影粉刺。
        isect->pError = gamma(5) * Vector3f(std::abs(pHit.x), std::abs(pHit.y), std::abs(pHit.z)) +
                        Vector3f(ShadowEpsilon, ShadowEpsilon, ShadowEpsilon);
        isect->wo = -ray.d;
        isect->n = Normal3f(normal.x, normal.y, normal.z);
        isect->uv = Point2f(phi * Inv2Pi, theta * InvPi);
        isect->dpdu = dpdu;
        isect->dpdv = dpdv;
        isect->dndu = Normal3f(0, 0, 0);
        isect->dndv = Normal3f(0, 0, 0);
        isect->shape = this;
        isect->shading.n = isect->n;
        isect->shading.dpdu = dpdu;
        isect->shading.dpdv = dpdv;
        isect->shading.dndu = Normal3f(0, 0, 0);
        isect->shading.dndv = Normal3f(0, 0, 0);
    }
    return true;
}

bool Sphere::IntersectP(const Ray &ray, bool testAlphaTexture) const
{
    Ray copy = ray;
    Float t = 0;
    return Intersect(copy, &t, nullptr, testAlphaTexture);
}

Interaction Sphere::Sample(const Point2f &u, Float *pdf) const
{
    Float z = 1 - 2 * u.x;
    Float r = std::sqrt(std::max((Float)0, 1 - z * z));
    Float phi = 2 * Pi * u.y;
    Vector3f dir(r * std::cos(phi), r * std::sin(phi), z);
    Point3f p = center + radius * dir;

    Interaction it;
    it.p = p;
    it.time = 0;
    it.pError = Vector3f(0, 0, 0);
    it.n = Normal3f(dir.x, dir.y, dir.z);
    if (pdf)
        *pdf = 1 / Area();
    return it;
}

} // namespace pbr
