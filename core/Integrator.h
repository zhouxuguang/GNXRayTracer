
#ifndef PBR_CORE_INTEGRATOR_H
#define PBR_CORE_INTEGRATOR_H

// core/integrator.h*
#include "Primitive.h"
#include "Spectrum.h"
//#include "light.h"
//#include "reflection.h"
#include "Sampler.h"
#include <functional>

namespace pbr
{

// 积分器只计算辐射亮度，像素结果通过回调交给调用方决定如何输出
using PixelResultCallback = std::function<void(int x, int y, const Spectrum &L)>;

// Integrator Declarations
class Integrator
{
public:
    // Integrator Interface
    virtual ~Integrator();
    // 渲染像素 (x, y)：相机、像素坐标与内存池都由外部传入，积分器本身无渲染状态
    virtual void Render(const Scene &scene, const Camera &camera, int x, int y,
                        MemoryArena &arena, double &timeConsume,
                        const PixelResultCallback &onPixel = nullptr) = 0;

	//进度函数
	virtual void Preprocess(const Scene& scene)
	{
	}
};

Spectrum UniformSampleAllLights(const Interaction &it, const Scene &scene,
                                MemoryArena &arena, Sampler &sampler,
                                const std::vector<int> &nLightSamples,
                                bool handleMedia = false);
Spectrum UniformSampleOneLight(const Interaction &it, const Scene &scene,
                               MemoryArena &arena, Sampler &sampler,
                               bool handleMedia = false,
                               const Distribution1D *lightDistrib = nullptr);
Spectrum EstimateDirect(const Interaction &it, const Point2f &uShading,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        MemoryArena &arena, bool handleMedia = false,
                        bool specular = false);

std::unique_ptr<Distribution1D> ComputeLightPowerDistribution(const Scene &scene);

// 简单的积分器
class SamplerIntegrator : public Integrator
{
public:
    // SamplerIntegrator Public Methods
    SamplerIntegrator(std::shared_ptr<Sampler> sampler)
        : mSampler(sampler)
    {
    }

    //渲染函数
    void Render(const Scene &scene, const Camera &camera, int x, int y,
                MemoryArena &arena, double &timeConsume,
                const PixelResultCallback &onPixel = nullptr) override;

    virtual Spectrum Li(const RayDifferential &ray, const Scene &scene,
                        Sampler &sampler, MemoryArena &arena,
                        int depth = 0) const = 0;

    Spectrum SpecularReflect(const RayDifferential &ray,
                             const SurfaceInteraction &isect,
                             const Scene &scene, Sampler &sampler,
                             MemoryArena &arena, int depth) const;

    Spectrum SpecularTransmit(const RayDifferential &ray,
                              const SurfaceInteraction &isect,
                              const Scene &scene, Sampler &sampler,
                              MemoryArena &arena, int depth) const;

protected:
    // SamplerIntegrator Protected Data
    // 只保留采样器：相机与像素尺寸都由调用方（Renderer）在 Render 时传入
    // 子类 Preprocess 也需要它（如 UniformSampleAll 预留样本维度）
    std::shared_ptr<Sampler> mSampler;
};

} // namespace pbr

#endif // PBR_CORE_INTEGRATOR_H
