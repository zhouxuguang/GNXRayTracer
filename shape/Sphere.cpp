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
    // 将射线转换到局部坐标系
    Ray rayLocal = (*WorldToObject)(ray);

    // 计算一元二次方程的系数
    Float ox(rayLocal.o.x), oy(rayLocal.o.y), oz(rayLocal.o.z);
    Float dx(rayLocal.d.x), dy(rayLocal.d.y), dz(rayLocal.d.z);
    Float a = dx * dx + dy * dy + dz * dz;
    Float b = 2 * (dx * ox + dy * oy + dz * oz);
    Float c = ox * ox + oy * oy + oz * oz - (radius * radius);
    
    // 解出t
    Float t0, t1;
    if (!Quadratic(a, b, c, &t0, &t1)) return false;

    // 检查最近的交点
    if (t0 > rayLocal.tMax || t1 <= 0) return false;
    Float tShapeHit = t0;
    if (tShapeHit <= 0)
    {
        tShapeHit = t1;
        if (tShapeHit > rayLocal.tMax) return false;
    }
    
    if (tHit)
    {
        *tHit = tShapeHit;
    }

    if (isect)
    {
        // 计算局部空间中的交点位置和法线
        Vector3f pHit = Vector3f(rayLocal.o + tShapeHit * rayLocal.d); // 交点位置（局部空间）
        Vector3f n = Normalize(pHit); // 交点法线（局部空间）
        
        // 计算切线和副切线方向
        Float zRadius = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
        Float invZRadius = 1 / zRadius;
        Float cosPhi = pHit.x * invZRadius;
        Float sinPhi = pHit.y * invZRadius;
        Vector3f dpdu(-pHit.y, pHit.x, 0);
        
        float theta = std::acosf(n.z);
        Vector3f dpdv = (-Pi) * Vector3f(pHit.z * cosPhi, pHit.z * sinPhi, -radius * std::sin(theta));
        
        dpdu = (*ObjectToWorld)(dpdu);
        dpdv = (*ObjectToWorld)(dpdv);

        isect->p = (*ObjectToWorld)(Point3f(pHit.x, pHit.y, pHit.z));
        isect->time = rayLocal.time;
        // 交点误差界：非零才能驱动 OffsetRayOrigin 把阴影射线推离球面，
        // 否则起点精确落在表面上会自相交，产生阴影粉刺。
        isect->pError = gamma(5) * Vector3f(std::abs(pHit.x), std::abs(pHit.y), std::abs(pHit.z)) +
                        Vector3f(ShadowEpsilon, ShadowEpsilon, ShadowEpsilon);
        isect->wo = -ray.d;
        isect->n = (*ObjectToWorld)(Normal3f(n.x, n.y, n.z));
        
        // 计算交点处的uv坐标
        float phi = std::atan2f(pHit.y, pHit.x); // [-pi, pi]
        if (phi < 0)
        {
            phi += 2 * Pi; // [0, 2pi]
        }
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
