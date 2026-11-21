
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
                  const std::shared_ptr<Texture<Float>> &bumpMap,
                  const std::shared_ptr<Texture<Spectrum>> &normalMap = nullptr)
        : mKd(Kd), mSigma(sigma), mBumpMap(bumpMap), mNormalMap(normalMap)
    {
    }
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    std::shared_ptr<Texture<Spectrum>> mKd;      // diffuse reflectance
    std::shared_ptr<Texture<Float>> mSigma, mBumpMap;  // 粗糙度和凹凸贴图
    std::shared_ptr<Texture<Spectrum>> mNormalMap;      // 法线贴图
};

} // namespace pbr

#endif // PBR_MATERIALS_MATTE_H
