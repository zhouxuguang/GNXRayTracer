
#ifndef PBR_MATERIALS_MIRROR_H
#define PBR_MATERIALS_MIRROR_H

// materials/mirror.h*
#include "core/Material.h"

namespace pbr
{

// MirrorMaterial Declarations
class MirrorMaterial : public Material
{
public:
    // MirrorMaterial Public Methods
    MirrorMaterial(const std::shared_ptr<Texture<Spectrum>> &r,
                   const std::shared_ptr<Texture<Float>> &bump)
    {
        Kr = r;
        bumpMap = bump;
    }
    
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    // MirrorMaterial Private Data
    std::shared_ptr<Texture<Spectrum>> Kr;
    std::shared_ptr<Texture<Float>> bumpMap;
};

}  // namespace pbrt

#endif  // PBRT_MATERIALS_MIRROR_H
