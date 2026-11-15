//
//  DisneyMaterial.h
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/7/2.
//

#ifndef PBR_DISNEY_MATERIAL_INCLUDE_H
#define PBR_DISNEY_MATERIAL_INCLUDE_H

#include "core/Material.h"

namespace pbr
{

// Disney²ÄÖÊ
class DisneyMaterial : public Material
{
public:
    DisneyMaterial(const std::shared_ptr<Texture<Spectrum>> &color,
                   const std::shared_ptr<Texture<Float>> &metallic,
                   const std::shared_ptr<Texture<Float>> &eta,
                   const std::shared_ptr<Texture<Float>> &roughness,
                   const std::shared_ptr<Texture<Float>> &specularTint,
                   const std::shared_ptr<Texture<Float>> &anisotropic,
                   const std::shared_ptr<Texture<Float>> &sheen,
                   const std::shared_ptr<Texture<Float>> &sheenTint,
                   const std::shared_ptr<Texture<Float>> &clearcoat,
                   const std::shared_ptr<Texture<Float>> &clearcoatGloss,
                   const std::shared_ptr<Texture<Float>> &specTrans,
                   const std::shared_ptr<Texture<Spectrum>> &scatterDistance,
                   bool thin,
                   const std::shared_ptr<Texture<Float>> &flatness,
                   const std::shared_ptr<Texture<Float>> &diffTrans,
                   const std::shared_ptr<Texture<Float>> &bumpMap)
        : mColor(color),
          mMetallic(metallic),
          mEta(eta),
          mRoughness(roughness),
          mSpecularTint(specularTint),
          mAnisotropic(anisotropic),
          mSheen(sheen),
          mSheenTint(sheenTint),
          mClearcoat(clearcoat),
          mClearcoatGloss(clearcoatGloss),
          mSpecTrans(specTrans),
          mScatterDistance(scatterDistance),
          mThin(thin),
          mFlatness(flatness),
          mDiffTrans(diffTrans),
          mBumpMap(bumpMap)
    {
    }
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    std::shared_ptr<Texture<Spectrum>> mColor;
    std::shared_ptr<Texture<Float>> mMetallic, mEta;
    std::shared_ptr<Texture<Float>> mRoughness, mSpecularTint, mAnisotropic, mSheen;
    std::shared_ptr<Texture<Float>> mSheenTint, mClearcoat, mClearcoatGloss;
    std::shared_ptr<Texture<Float>> mSpecTrans;
    std::shared_ptr<Texture<Spectrum>> mScatterDistance;
    bool mThin;
    std::shared_ptr<Texture<Float>> mFlatness, mDiffTrans, mBumpMap;
};

} // namespace pbr

#endif /* PBR_DISNEY_MATERIAL_INCLUDE_H */
