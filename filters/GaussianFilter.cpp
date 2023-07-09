// filters/gaussian.cpp*
#include "GaussianFilter.h"

namespace pbr
{

// Gaussian Filter Method Definitions
Float GaussianFilter::Evaluate(const Point2f &p) const
{
    return Gaussian(p.x, expX) * Gaussian(p.y, expY);
}


}  // namespace pbr
