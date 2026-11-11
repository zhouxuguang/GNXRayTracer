
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
                      uint32_t width, uint32_t height, Float rrThreshold = 1,
                      const std::string &lightSampleStrategy = "spatial")
        : SamplerIntegrator(camera, sampler, width, height),
          maxDepth(maxDepth),
          rrThreshold(rrThreshold),
          lightSampleStrategy(lightSampleStrategy)
    {
    }
    void Preprocess(const Scene &scene) override;
    Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;

private:
    // VolPathIntegrator Private Data
    const int maxDepth;
    const Float rrThreshold;
    const std::string lightSampleStrategy;
    std::unique_ptr<LightDistribution> lightDistribution;
};

} // namespace pbr

#endif // PBR_INTEGRATORS_VOLPATH_H
