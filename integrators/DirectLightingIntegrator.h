
#ifndef PBR_INTEGRATORS_DIRECTLIGHTING_H_INCLUDE
#define PBR_INTEGRATORS_DIRECTLIGHTING_H_INCLUDE

// integrators/directlighting.h*
#include "core/Integrator.h"
#include "core/Scene.h"

namespace pbr
{

// LightStrategy Declarations
enum class LightStrategy
{
    UniformSampleAll,
    UniformSampleOne
};

// DirectLightingIntegrator Declarations
class DirectLightingIntegrator : public SamplerIntegrator
{
public:
    // DirectLightingIntegrator Public Methods
    DirectLightingIntegrator(LightStrategy strategy, int maxDepth,
                             std::shared_ptr<const Camera> camera,
                             std::shared_ptr<Sampler> sampler,
                             uint32_t width, uint32_t height)
        : SamplerIntegrator(camera, sampler, width, height),
          strategy(strategy),
          maxDepth(maxDepth)
    {
    }
    Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;
    void Preprocess(const Scene &scene) override;

private:
    // DirectLightingIntegrator Private Data
    const LightStrategy strategy;
    const int maxDepth;
    std::vector<int> nLightSamples;
};

} // namespace pbr

#endif // PBR_INTEGRATORS_DIRECTLIGHTING_H_INCLUDE
