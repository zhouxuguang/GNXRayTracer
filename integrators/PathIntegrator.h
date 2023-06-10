
#ifndef PBR_INTEGRATORS_PATH_INCLUDE_H
#define PBR_INTEGRATORS_PATH_INCLUDE_H

// integrators/path.h*
#include "core/Integrator.h"
#include "lights/LightDistribution.h"

namespace pbr
{

// PathIntegrator Declarations
class PathIntegrator : public SamplerIntegrator
{
public:
    // PathIntegrator Public Methods
    PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
                   std::shared_ptr<Sampler> sampler,
                   const Bounds2i &pixelBounds, FrameBuffer * pFrameBuffer, Float rrThreshold = 1,
                   const std::string &lightSampleStrategy = "spatial");

    void Preprocess(const Scene &scene, Sampler &sampler);
    
    Spectrum Li(const Ray &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;

private:
    // PathIntegrator Private Data
    const int maxDepth;
    const Float rrThreshold;
    const std::string lightSampleStrategy;
    std::unique_ptr<LightDistribution> lightDistribution;
};

}  // namespace pbr

#endif  // PBR_INTEGRATORS_PATH_INCLUDE_H
