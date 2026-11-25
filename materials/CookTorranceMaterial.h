//
//  CookTorranceMaterial.h
//  GNXRayTracer
//
//  Created by zhouxuguang on 2026/11/23.
//

#ifndef PBR_COOK_TORRANCE_MATERIAL_INCLUDE_H
#define PBR_COOK_TORRANCE_MATERIAL_INCLUDE_H

#include "core/Material.h"

namespace pbr
{

// CookTorrance材质
class CookTorranceMaterial : public Material
{
public:
	CookTorranceMaterial(const std::shared_ptr<Texture<Spectrum>>& albedo,
		const std::shared_ptr<Texture<Float>>& metallic,
		const std::shared_ptr<Texture<Float>>& roughness,
		const std::shared_ptr<Texture<Float>>& bumpMap,
		const std::shared_ptr<Texture<Spectrum>>& normalMap = nullptr,
		Float bumpScale = 1.0)
		: mAlbedo(albedo),
		mMetallic(metallic),
		mRoughness(roughness),
		mBumpMap(bumpMap),
		mNormalMap(normalMap),
		mBumpScale(bumpScale)
	{
	}

	void ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena,
		TransportMode mode,
		bool allowMultipleLobes) const;

private:
	std::shared_ptr<Texture<Spectrum>> mAlbedo;         //反照率
	std::shared_ptr<Texture<Float>> mMetallic;          //金属度
	std::shared_ptr<Texture<Float>> mRoughness;         //粗糙度
	std::shared_ptr<Texture<Float>> mBumpMap;           //凹凸贴图
	std::shared_ptr<Texture<Spectrum>> mNormalMap;      //法线贴图
	Float mBumpScale;                                   //法线贴图缩放系数
};

}

#endif // PBR_COOK_TORRANCE_MATERIAL_INCLUDE_H
