
#ifndef PBR_MATERIALS_MATTE_H
#define PBR_MATERIALS_MATTE_H

// materials/matte.h*
#include "core/Material.h"

namespace pbr
{

// MatteMaterial Declarations
class MatteMaterial : public Material
{
public:
    // MatteMaterial Public Methods
    MatteMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd,
                  const std::shared_ptr<Texture<Float>> &sigma,
                  const std::shared_ptr<Texture<Float>> &bumpMap)
        : Kd(Kd), sigma(sigma), bumpMap(bumpMap) {}
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    // MatteMaterial Private Data
    std::shared_ptr<Texture<Spectrum>> Kd;
    std::shared_ptr<Texture<Float>> sigma, bumpMap;
};

}  // namespace pbr

#endif  // PBR_MATERIALS_MATTE_H
