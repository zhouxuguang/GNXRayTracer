
// materials/mirror.cpp*
#include "MirrorMaterial.h"
#include "core/Spectrum.h"
#include "core/Reflection.h"
#include "core/Texture.h"
#include "core/Interaction.h"

namespace pbr
{

// MirrorMaterial Method Definitions
void MirrorMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                                MemoryArena &arena,
                                                TransportMode mode,
                                                bool allowMultipleLobes) const
{
    // Perform bump mapping with _bumpMap_, if present
    if (bumpMap) Bump(bumpMap, si);
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum R = Kr->Evaluate(*si).Clamp();
    if (!R.IsBlack())
        si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(R, ARENA_ALLOC(arena, FresnelNoOp)()));
}

}  // namespace pbrt
