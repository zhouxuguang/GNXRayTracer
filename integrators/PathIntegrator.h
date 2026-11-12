
#ifndef PBR_INTEGRATORS_PATH_INCLUDE_H
#define PBR_INTEGRATORS_PATH_INCLUDE_H

// integrators/path.h*
#include "core/Integrator.h"
#include "core/LightDistribution.h"

namespace pbr
{

// 路径追踪积分器
class PathIntegrator : public SamplerIntegrator
{
public:
    // PathIntegrator Public Methods
    PathIntegrator(int maxDepth, std::shared_ptr<Sampler> sampler,
                   Float rrThreshold = 1,
                   const std::string &lightSampleStrategy = "spatial");

    void Preprocess(const Scene &scene) override;

    Spectrum Li(const RayDifferential &ray, const Scene &scene,
                Sampler &sampler, MemoryArena &arena, int depth) const override;

private:
    // PathIntegrator Private Data
    const int mMaxDepth;
    const Float mRRThreshold;
    const std::string mLightSampleStrategy;
    std::unique_ptr<LightDistribution> mLightDistribution;
};

} // namespace pbr

#endif // PBR_INTEGRATORS_PATH_INCLUDE_H
