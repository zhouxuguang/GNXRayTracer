

#ifndef PBR_MATERIALS_METAL_INCLUDE_H
#define PBR_MATERIALS_METAL_INCLUDE_H

// materials/metal.h*
#include "core/Material.h"
#include "core/Spectrum.h"

namespace pbr 
{

// MetalMaterial Declarations
class MetalMaterial : public Material {
  public:
    // MetalMaterial Public Methods
    MetalMaterial(const std::shared_ptr<Texture<Spectrum>> &eta,
                  const std::shared_ptr<Texture<Spectrum>> &k,
                  const std::shared_ptr<Texture<Float>> &rough,
                  const std::shared_ptr<Texture<Float>> &urough,
                  const std::shared_ptr<Texture<Float>> &vrough,
                  const std::shared_ptr<Texture<Float>> &bump,
                  bool remapRoughness);
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    // MetalMaterial Private Data
    std::shared_ptr<Texture<Spectrum>> eta, k;
    std::shared_ptr<Texture<Float>> roughness, uRoughness, vRoughness;
    std::shared_ptr<Texture<Float>> bumpMap;
    bool remapRoughness;
};

}  // namespace pbr

#endif  // PBR_MATERIALS_METAL_INCLUDE_H
