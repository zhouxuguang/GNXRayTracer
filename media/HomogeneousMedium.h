
#ifndef PBR_MEDIA_HOMOGENEOUS_H
#define PBR_MEDIA_HOMOGENEOUS_H

// media/homogeneous.h*
#include "core/Medium.h"

namespace pbr
{

// HomogeneousMedium Declarations
class HomogeneousMedium : public Medium
{
public:
    // HomogeneousMedium Public Methods
    HomogeneousMedium(const Spectrum &sigma_a, const Spectrum &sigma_s, Float g)
        : sigma_a(sigma_a),
          sigma_s(sigma_s),
          sigma_t(sigma_s + sigma_a),
          g(g) {}
    Spectrum Tr(const Ray &ray, Sampler &sampler) const;
    Spectrum Sample(const Ray &ray, Sampler &sampler, MemoryArena &arena,
                    MediumInteraction *mi) const;

private:
    // HomogeneousMedium Private Data
    const Spectrum sigma_a, sigma_s, sigma_t;
    const Float g;
};

}  // namespace pbr

#endif  // PBR_MEDIA_HOMOGENEOUS_H
