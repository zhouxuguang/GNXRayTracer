#ifndef __PBR_SPHEREH__
#define __PBR_SPHEREH__

#include "core/Shape.h"

namespace pbr
{

// 球体定义
class Sphere : public Shape
{
  public:
    // Sphere Public Methods
    Sphere(const Transform *ObjectToWorld, const Transform *WorldToObject,
           bool reverseOrientation, float radius)
        : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
          radius(radius) {}
    Bounds3f ObjectBound() const;
    bool Intersect(const Ray &ray, float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const;
    bool IntersectP(const Ray &ray, bool testAlphaTexture) const;
    float Area() const;
private:
    // Sphere Private Data
    const float radius;
};

bool Sphere::Intersect(const Ray &r, float *tHit, SurfaceInteraction *isect, bool testAlphaTexture) const
{

    Point3f pHit;
    // Transform _Ray_ to object space
    Ray ray = (*WorldToObject)(r);

    Vector3f oc = ray.o - Point3f(0.0f,0.0f,0.0f);
    float a = Dot(ray.d, ray.d);
    float b = 2.0 * Dot(oc, ray.d);
    float c = Dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4 * a*c;

    return (discriminant > 0);
}

bool Sphere::IntersectP(const Ray &r, bool testAlphaTexture) const
{
    return false;
}

}

#endif


















