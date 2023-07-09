
#ifndef PBR_CORE_FILTER_H
#define PBR_CORE_FILTER_H

// core/filter.h*
#include "Geometry.h"

namespace pbr
{

// 滤波类定义
class Filter
{
public:
    // Filter Interface
    virtual ~Filter();
    Filter(const Vector2f &radius)
        : radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y)) {}
    virtual Float Evaluate(const Point2f &p) const = 0;

    // Filter Public Data
    const Vector2f radius, invRadius;
};

}  // namespace pbr

#endif  // PBR_CORE_FILTER_H
