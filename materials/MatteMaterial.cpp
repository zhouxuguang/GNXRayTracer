


// materials/matte.cpp*
#include "MatteMaterial.h"
#include "core/Reflection.h"
#include "core/Interaction.h"
#include "core/Texture.h"

namespace pbr
{

// MatteMaterial Method Definitions
void MatteMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                               MemoryArena &arena,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const
{
    // Perform bump mapping with _bumpMap_, if present
    if (mBumpMap)
    {
        Bump(mBumpMap, si);
    }
    
    if (mNormalMap)
    {
        NormalMap(mNormalMap, si, mBumpScale);
    }

    // Evaluate textures for _MatteMaterial_ material and allocate BRDF
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum r = mKd->Evaluate(*si).Clamp();
    // sigma 为弧度，上界取 90 度
    Float sig = Clamp(mSigma->Evaluate(*si), 0, PiOver2);
    if (!r.IsBlack())
    {
        if (sig == 0)
            si->bsdf->Add(ARENA_ALLOC(arena, LambertianReflection)(r));
        else
            si->bsdf->Add(ARENA_ALLOC(arena, OrenNayar)(r, sig));
    }
}

} // namespace pbr
