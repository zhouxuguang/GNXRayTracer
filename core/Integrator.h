
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
    // 渲染像素 (x, y)：整数像素坐标，结果通过 onPixel 交给调用方
    virtual void Render(const Scene &scene, int x, int y, double &timeConsume,
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
    SamplerIntegrator(std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
        uint32_t width, uint32_t height)
        : mCamera(camera), mSampler(sampler), mWidth(width), mHeight(height)
    {
    }

    //渲染函数
    void Render(const Scene &scene, int x, int y, double &timeConsume,
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
    std::shared_ptr<const Camera> mCamera;
    // 子类 Preprocess 可能需要访问采样器（如 UniformSampleAll 预留样本维度）
    std::shared_ptr<Sampler> mSampler;

private:
    // SamplerIntegrator Private Data
    const uint32_t mWidth, mHeight;
};

} // namespace pbr

#endif // PBR_CORE_INTEGRATOR_H
