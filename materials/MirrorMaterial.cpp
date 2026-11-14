
// materials/mirror.cpp*
#include "MirrorMaterial.h"
#include "core/Spectrum.h"
#include "core/Reflection.h"
#include "core/Texture.h"
#include "core/Interaction.h"

namespace pbr
{

void MirrorMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                                MemoryArena &arena,
                                                TransportMode mode,
                                                bool allowMultipleLobes) const
{
    if (bumpMap)
        Bump(bumpMap, si);
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum R = mKr->Evaluate(*si).Clamp();
    if (R.IsBlack())
        return;

    Fresnel *fresnel;
    if (mEta && mK)
        fresnel = ARENA_ALLOC(arena, FresnelConductor)(mEta->Evaluate(*si),
                                                      mK->Evaluate(*si));
    else
        fresnel = ARENA_ALLOC(arena, FresnelNoOp)();
    si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(R, fresnel));
}

} // namespace pbr
