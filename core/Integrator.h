
#ifndef PBR_CORE_INTEGRATOR_H
#define PBR_CORE_INTEGRATOR_H

// core/integrator.h*
#include "Primitive.h"
#include "Spectrum.h"
//#include "light.h"
//#include "reflection.h"
#include "Sampler.h"

namespace pbr
{

// Integrator Declarations
class Integrator
{
  public:
    // Integrator Interface
    virtual ~Integrator();
    virtual void Render(const Scene &scene) = 0;
};

// SamplerIntegrator Declarations
class SamplerIntegrator : public Integrator
{
  public:
    // SamplerIntegrator Public Methods
    SamplerIntegrator(std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
                      const Bounds2i &pixelBounds) : camera(camera), sampler(sampler), pixelBounds(pixelBounds)
    {}
    
    //进度函数
    virtual void Preprocess(const Scene &scene, Sampler &sampler) {}
    
    //渲染函数
    void Render(const Scene &scene);
    
protected:
    // SamplerIntegrator Protected Data
    std::shared_ptr<const Camera> camera;

private:
    // SamplerIntegrator Private Data
    std::shared_ptr<Sampler> sampler;
    const Bounds2i pixelBounds;
};

}  // namespace pbr

#endif  // PBR_CORE_INTEGRATOR_H
