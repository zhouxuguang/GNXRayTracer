
#ifndef PBR_SAMPLERS_HALTON_H
#define PBR_SAMPLERS_HALTON_H

// samplers/halton.h*
#include "core/Sampler.h"
#include "LowDiscrepancy.h"

namespace pbr
{
class HaltonSampler : public GlobalSampler
{
public:
    HaltonSampler(int nsamp, const Bounds2i &sampleBounds,
                  bool sampleAtCenter = false);
    int64_t GetIndexForSample(int64_t sampleNum) const;
    Float SampleDimension(int64_t index, int dimension) const;
    std::unique_ptr<Sampler> Clone(int seed);

private:
    static std::vector<uint16_t> radicalInversePermutations;
    Point2i baseScales, baseExponents;
    int sampleStride;
    int multInverse[2];
    mutable Point2i pixelForOffset = Point2i(std::numeric_limits<int>::max(),
                                             std::numeric_limits<int>::max());
    mutable int64_t offsetForCurrentPixel;
    // Added after book publication: force all image samples to be at the
    // center of the pixel area.
    bool sampleAtPixelCenter;

    const uint16_t *PermutationForDimension(int dim) const
    {
        return &radicalInversePermutations[PrimeSums[dim]];
    }
};

} // namespace pbr

#endif // PBR_SAMPLERS_HALTON_H
