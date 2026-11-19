#include "Geometry.h"

namespace pbr
{

Bounds2iIterator begin(const Bounds2i &b)
{
    return Bounds2iIterator(b, b.pMin);
}

Bounds2iIterator end(const Bounds2i &b)
{
    // Normally, the ending point is at the minimum x value and one past
    // the last valid y value.
    Point2i pEnd(b.pMin.x, b.pMax.y);
    // However, if the bounds are degenerate, override the end point to
    // equal the start point so that any attempt to iterate over the bounds
    // exits out immediately.
    if (b.pMin.x >= b.pMax.x || b.pMin.y >= b.pMax.y)
        pEnd = b.pMin;
    return Bounds2iIterator(b, pEnd);
}

Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi)
{
    return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
                    cosTheta);
}

Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi,
                            const Vector3f &x, const Vector3f &y,
                            const Vector3f &z)
{
    return sinTheta * std::cos(phi) * x + sinTheta * std::sin(phi) * y +
           cosTheta * z;
}

Float SphericalTheta(const Vector3f &v)
{
    return std::acos(Clamp(v.z, -1, 1));
}

Float SphericalPhi(const Vector3f &v)
{
    Float p = std::atan2(v.y, v.x);
    return (p < 0) ? (p + 2 * Pi) : p;
}

} // namespace pbr
