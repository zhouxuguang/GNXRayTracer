#include "Sphere.h"
#include "core/Interaction.h"
#include <algorithm>
#include <cmath>

namespace pbr
{

namespace
{
const Transform &Identity()
{
    static const Transform identity;
    return identity;
}
} // namespace

Sphere::Sphere(Float radius, const Point3f &center)
    : Shape(&Identity(), &Identity(), false),
      radius(radius),
      center(center)
{
}

Bounds3f Sphere::ObjectBound() const
{
    return Bounds3f(Point3f(center.x - radius, center.y - radius, center.z - radius),
                    Point3f(center.x + radius, center.y + radius, center.z + radius));
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
        Vector3f tangent = Cross(Vector3f(0, 0, 1), rel);
        if (tangent.LengthSquared() < 1e-12f)
            tangent = Cross(Vector3f(0, 1, 0), rel);
        Vector3f bitangent = Cross(rel, tangent);
        Vector3f normal = Normalize(Cross(tangent, bitangent));

        Float phi = std::atan2(rel.y, rel.x);
        if (phi < 0)
            phi += 2 * Pi;
        Float theta = std::acos(Clamp(rel.z / radius, -1.f, 1.f));

        isect->p = pHit;
        isect->time = ray.time;
        isect->pError = Vector3f(0, 0, 0);
        isect->wo = -ray.d;
        isect->n = Normal3f(normal.x, normal.y, normal.z);
        isect->uv = Point2f(phi / (2 * Pi), theta / Pi);
        isect->dpdu = tangent;
        isect->dpdv = bitangent;
        isect->dndu = Normal3f(0, 0, 0);
        isect->dndv = Normal3f(0, 0, 0);
        isect->shape = this;
        isect->shading.n = isect->n;
        isect->shading.dpdu = tangent;
        isect->shading.dpdv = bitangent;
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
