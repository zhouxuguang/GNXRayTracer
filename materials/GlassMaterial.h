

#ifndef PBR_MATERIALS_GLASS_INCLUDE_H
#define PBR_MATERIALS_GLASS_INCLUDE_H

// materials/glass.h*
#include "core/Material.h"

namespace pbr
{

// 玻璃材质
class GlassMaterial : public Material
{
public:
    GlassMaterial(const std::shared_ptr<Texture<Spectrum>> &Kr,
                  const std::shared_ptr<Texture<Spectrum>> &Kt,
                  const std::shared_ptr<Texture<Float>> &uRoughness,
                  const std::shared_ptr<Texture<Float>> &vRoughness,
                  const std::shared_ptr<Texture<Float>> &index,
                  const std::shared_ptr<Texture<Float>> &bumpMap,
                  const std::shared_ptr<Texture<Spectrum>> &normalMap = nullptr,
                  Float bumpScale = 1.f,
                  bool remapRoughness = true)
        : mKr(Kr),
          mKt(Kt),
          mURoughness(uRoughness),
          mVRoughness(vRoughness),
          mIndex(index),
          mBumpMap(bumpMap),
          mNormalMap(normalMap),
          mBumpScale(bumpScale),
          mRemapRoughness(remapRoughness)
    {
    }
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    std::shared_ptr<Texture<Spectrum>> mKr, mKt;
    std::shared_ptr<Texture<Float>> mURoughness, mVRoughness;
    std::shared_ptr<Texture<Float>> mIndex;
    std::shared_ptr<Texture<Float>> mBumpMap;
    std::shared_ptr<Texture<Spectrum>> mNormalMap;
    Float mBumpScale;
    bool mRemapRoughness;
};

} // namespace pbrt

#endif // PBRT_MATERIALS_GLASS_H
