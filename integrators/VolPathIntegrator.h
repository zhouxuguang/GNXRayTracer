
#ifndef PBR_INTEGRATORS_VOLPATH_H
#define PBR_INTEGRATORS_VOLPATH_H

// integrators/volpath.h*
#include "core/Integrator.h"
#include "core/LightDistribution.h"

namespace pbr
{

// VolPathIntegrator Declarations
class VolPathIntegrator : public SamplerIntegrator
{
public:
    // VolPathIntegrator Public Methods
    VolPathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
                      const Bounds2i &pixelBounds, Float rrThreshold = 1,
                      const std::string &lightSampleStrategy = "spatial", FrameBuffer * pFrameBuffer = nullptr)
        : SamplerIntegrator(camera, sampler, pixelBounds, pFrameBuffer),
          maxDepth(maxDepth),
          rrThreshold(rrThreshold),
          lightSampleStrategy(lightSampleStrategy) { }
    void Preprocess(const Scene &scene, Sampler &sampler);
    Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;

private:
    // VolPathIntegrator Private Data
    const int maxDepth;
    const Float rrThreshold;
    const std::string lightSampleStrategy;
    std::unique_ptr<LightDistribution> lightDistribution;
};

}  // namespace pbr

#endif  // PBR_INTEGRATORS_VOLPATH_H
