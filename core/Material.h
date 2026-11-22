

#ifndef PBR_CORE_MATERIAL_H
#define PBR_CORE_MATERIAL_H

// core/material.h*
#include "GNXRayTracer.h"
#include "Memory.h"

namespace pbr
{

enum class TransportMode
{
    Radiance,
    Importance
};

// 材质基类
class Material
{
public:
    virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
    virtual ~Material();
    static void Bump(const std::shared_ptr<Texture<Float>> &d, SurfaceInteraction *si);
    
    static void NormalMap(const std::shared_ptr<Texture<Spectrum>> &normalMap,
                          SurfaceInteraction *si, Float bumpScale = 1.f);
};

} // namespace pbr

#endif // PBR_CORE_MATERIAL_H
