

// materials/glass.cpp*
#include "GlassMaterial.h"
#include "core/Spectrum.h"
#include "core/Reflection.h"
#include "core/Texture.h"
#include "core/Interaction.h"

namespace pbr
{

// GlassMaterial Method Definitions
void GlassMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const
{
    // Perform bump mapping with _bumpMap_, if present
    if (mBumpMap)
        Bump(mBumpMap, si);
    // 法线贴图必须作用在 BSDF 构造之前，否则着色法线不会进入 _bsdf_ 的局部坐标系
    if (mNormalMap)
        NormalMap(mNormalMap, si, mBumpScale);
    Float eta = mIndex->Evaluate(*si);
    Float urough = mURoughness->Evaluate(*si);
    Float vrough = mVRoughness->Evaluate(*si);
    Spectrum R = mKr->Evaluate(*si).Clamp();
    Spectrum T = mKt->Evaluate(*si).Clamp();
    // Initialize _bsdf_ for smooth or rough dielectric
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si, eta);

    if (R.IsBlack() && T.IsBlack())
        return;

    bool isSpecular = urough == 0 && vrough == 0;
    if (isSpecular && allowMultipleLobes)
    {
        si->bsdf->Add(
            ARENA_ALLOC(arena, FresnelSpecular)(R, T, 1.f, eta, mode));
    }
    else
    {
        if (mRemapRoughness)
        {
            urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
            vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
        }
        MicrofacetDistribution *distrib =
            isSpecular ? nullptr
                       : ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(
                             urough, vrough);
        if (!R.IsBlack())
        {
            Fresnel *fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, eta);
            if (isSpecular)
                si->bsdf->Add(
                    ARENA_ALLOC(arena, SpecularReflection)(R, fresnel));
            else
                si->bsdf->Add(ARENA_ALLOC(arena, MicrofacetReflection)(
                    R, distrib, fresnel));
        }
        if (!T.IsBlack())
        {
            if (isSpecular)
                si->bsdf->Add(ARENA_ALLOC(arena, SpecularTransmission)(
                    T, 1.f, eta, mode));
            else
                si->bsdf->Add(ARENA_ALLOC(arena, MicrofacetTransmission)(
                    T, distrib, 1.f, eta, mode));
        }
    }
}

} // namespace pbr
