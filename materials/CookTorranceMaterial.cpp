#include "CookTorranceMaterial.h"
#include "core/Reflection.h"
#include "core/Spectrum.h"
#include "core/Texture.h"

namespace pbr
{

void CookTorranceMaterial::ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode, bool allowMultipleLobes) const
{
	if (mBumpMap)
	{
		Bump(mBumpMap, si);
	}

	if (mNormalMap)
	{
		NormalMap(mNormalMap, si, mBumpScale);
	}

	si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);

	Spectrum albedo = mAlbedo->Evaluate(*si).Clamp();
	Float roughness = Clamp(mRoughness->Evaluate(*si), 0.03f, 1.f);
	Float metallic = Clamp(mMetallic->Evaluate(*si), 0.f, 1.f);

	Float alpha = roughness * roughness;
	Spectrum F0 = Lerp(metallic, Spectrum(0.04f), albedo);           // F0 = mix(0.04, albedo, metallic)
	MicrofacetDistribution* distrib =
		ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(alpha, alpha);  // 各向同性
	Fresnel* fresnel = ARENA_ALLOC(arena, FresnelSchlick)(F0);

	si->bsdf->Add(ARENA_ALLOC(arena, CookTorranceBxDF)(
		albedo, metallic, roughness, distrib, fresnel));
}

}
