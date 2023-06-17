

#ifndef PBR_MATERIALS_PLASTIC_INCLUDE_H
#define PBR_MATERIALS_PLASTIC_INCLUDE_H

// materials/plastic.h*
#include "core/Material.h"

namespace pbr 
{

// PlasticMaterial Declarations
class PlasticMaterial : public Material 
{
public:
    // PlasticMaterial Public Methods
    PlasticMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd,
                    const std::shared_ptr<Texture<Spectrum>> &Ks,
                    const std::shared_ptr<Texture<Float>> &roughness,
                    const std::shared_ptr<Texture<Float>> &bumpMap,
                    bool remapRoughness)
        : Kd(Kd),
          Ks(Ks),
          roughness(roughness),
          bumpMap(bumpMap),
          remapRoughness(remapRoughness) {}
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    // PlasticMaterial Private Data
    std::shared_ptr<Texture<Spectrum>> Kd, Ks;
    std::shared_ptr<Texture<Float>> roughness, bumpMap;
    const bool remapRoughness;
};

}  // namespace pbr

#endif  // PBR_MATERIALS_PLASTIC_INCLUDE_H
