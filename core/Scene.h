#ifndef PBR_CORE_SCENE_H
#define PBR_CORE_SCENE_H

// core/scene.h*
#include "Geometry.h"
#include "Primitive.h"
//#include "light.h"

namespace pbr
{

// Scene Declarations
class Scene
{
public:
    // Scene Public Methods
    Scene(std::shared_ptr<Primitive> aggregate) : aggregate(aggregate)
    {
        // Scene Constructor Implementation
        worldBound = aggregate->WorldBound();
    }
    
    const Bounds3f &WorldBound() const { return worldBound; }
    bool Intersect(const Ray &ray, SurfaceInteraction *isect) const;
    bool IntersectP(const Ray &ray) const;
//    bool IntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect,
//                     Spectrum *transmittance) const;

//    // Scene Public Data
//    std::vector<std::shared_ptr<Light>> lights;
//    // Store infinite light sources separately for cases where we only want
//    // to loop over them.
//    std::vector<std::shared_ptr<Light>> infiniteLights;

private:
    // Scene Private Data
    std::shared_ptr<Primitive> aggregate;
    Bounds3f worldBound;
};

}  // namespace pbr

#endif  // PBR_CORE_SCENE_H