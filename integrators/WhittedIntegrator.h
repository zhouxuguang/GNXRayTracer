#ifndef PBR_INTEGRATORS_WHITTED_H
#define PBR_INTEGRATORS_WHITTED_H

// integrators/whitted.h*
#include "core/Integrator.h"
#include "core/Scene.h"

namespace pbr
{

// WhittedIntegrator Declarations
class WhittedIntegrator : public SamplerIntegrator
{
public:
    // WhittedIntegrator Public Methods
    WhittedIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
                      const Bounds2i &pixelBounds, FrameBuffer * pFrameBuffer)
        : SamplerIntegrator(camera, sampler, pixelBounds, pFrameBuffer), maxDepth(maxDepth) {}
    
    Spectrum Li(const Ray &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const;

private:
    // WhittedIntegrator Private Data
    const int maxDepth;
};

}  // namespace pbr

#endif  // PBR_INTEGRATORS_WHITTED_H
