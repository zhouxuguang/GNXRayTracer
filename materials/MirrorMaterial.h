
#ifndef PBR_MATERIALS_MIRROR_H
#define PBR_MATERIALS_MIRROR_H

// materials/mirror.h*
#include "core/Material.h"

namespace pbr
{

// 镜面反射材质类
class MirrorMaterial : public Material
{
public:
    // 纯镜面反射：菲涅尔恒为 1，反射率只由 Kr 决定
    MirrorMaterial(const std::shared_ptr<Texture<Spectrum>> &r,
                   const std::shared_ptr<Texture<Float>> &bump)
        : mKr(r), bumpMap(bump)
    {
    }

    // 导体镜面反射：用复折射率 (eta, k) 计算菲涅尔
    MirrorMaterial(const std::shared_ptr<Texture<Spectrum>> &r,
                   const std::shared_ptr<Texture<Spectrum>> &eta,
                   const std::shared_ptr<Texture<Spectrum>> &k,
                   const std::shared_ptr<Texture<Float>> &bump)
        : mKr(r), mEta(eta), mK(k), bumpMap(bump)
    {
    }

    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    std::shared_ptr<Texture<Spectrum>> mKr = nullptr;
    std::shared_ptr<Texture<Spectrum>> mEta = nullptr, mK = nullptr;
    std::shared_ptr<Texture<Float>> bumpMap = nullptr;
};

} // namespace pbr

#endif // PBR_MATERIALS_MIRROR_H
