

// integrators/directlighting.cpp*
#include "DirectLightingIntegrator.h"
#include "core/Interaction.h"
//#include "paramset.h"
#include "core/Camera.h"

namespace pbr
{

// DirectLightingIntegrator Method Definitions
void DirectLightingIntegrator::Preprocess(const Scene &scene)
{
    if (strategy == LightStrategy::UniformSampleAll)
    {
        // 每次 Preprocess 都重新统计，避免重复调用时样本数累加
        nLightSamples.clear();

        // Compute number of samples to use for each light
        for (const auto &light : scene.lights)
            nLightSamples.push_back(mSampler->RoundCount(light->nSamples));

        // Request samples for sampling all lights
        for (int i = 0; i < maxDepth; ++i)
        {
            for (size_t j = 0; j < scene.lights.size(); ++j)
            {
                mSampler->Request2DArray(nLightSamples[j]);
                mSampler->Request2DArray(nLightSamples[j]);
            }
        }
    }
}

#if 0

Spectrum DirectLightingIntegrator::Li(const RayDifferential &ray,
                                      const Scene &scene, Sampler &sampler,
                                      MemoryArena &arena, int depth) const
{
    Spectrum L(0.f);
    // Find closest ray intersection or return background radiance
    SurfaceInteraction isect;
    if (!scene.Intersect(ray, &isect))
    {
        for (const auto &light : scene.lights)
            L += light->Le(ray);
        return L;
    }

    // Compute scattering functions for surface interaction
    isect.ComputeScatteringFunctions(ray, arena);
    if (!isect.bsdf)
        return Li(isect.SpawnRay(ray.d), scene, sampler, arena, depth);
    Vector3f wo = isect.wo;
    // Compute emitted light if ray hit an area light source
    L += isect.Le(wo);
    if (scene.lights.size() > 0)
    {
        // Compute direct lighting for _DirectLightingIntegrator_ integrator
        if (strategy == LightStrategy::UniformSampleAll)
            L += UniformSampleAllLights(isect, scene, arena, sampler,
                                        nLightSamples);
        else
            L += UniformSampleOneLight(isect, scene, arena, sampler);
    }
    if (depth + 1 < maxDepth)
    {
        // Trace rays for specular reflection and refraction
        L += SpecularReflect(ray, isect, scene, sampler, arena, depth);
        L += SpecularTransmit(ray, isect, scene, sampler, arena, depth);
    }
    return L;
}

#else

Spectrum DirectLightingIntegrator::Li(const RayDifferential& ray,
    const Scene& scene, Sampler& sampler,
    MemoryArena& arena, int depth) const
{
    Spectrum E(0.f);

	pbr::SurfaceInteraction isect;
	if (!scene.Intersect(ray, &isect))
	{
		return E;
	}

	// 遍历各个光源，计算L
	for (auto light : scene.lights)
	{
		pbr::Point2f u;
		pbr::Vector3f wi;
		pbr::Float pdf;
		pbr::VisibilityTester visibility;
		pbr::Spectrum L = light->Sample_Li(isect, u, &wi, &pdf, &visibility);

		// 生成阴影射线，判断当前着色点是否在阴影中
		if (!visibility.Unoccluded(scene))
		{
			continue;
		}

		pbr::Float cosTheta = pbr::Dot(isect.shading.n, wi);

		E += L * std::max(cosTheta, 0.0f);
	}
	return E;
}

#endif

} // namespace pbr
