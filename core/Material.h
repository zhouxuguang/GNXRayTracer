

#ifndef PBR_CORE_MATERIAL_H
#define PBR_CORE_MATERIAL_H

// core/material.h*
#include "GNXRayTracer.h"
#include "Memory.h"

namespace pbr
{

// TransportMode Declarations
enum class TransportMode { Radiance, Importance };

// Material Declarations
class Material
{
public:
    // Material Interface
    virtual void ComputeScatteringFunctions(SurfaceInteraction *si,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
    virtual ~Material();
    static void Bump(const std::shared_ptr<Texture<Float>> &d,
                     SurfaceInteraction *si);
};

}  // namespace pbrt

#endif  // PBRT_CORE_MATERIAL_H
