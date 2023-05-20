

#ifndef PBR_TEXTURES_CONSTANT_H
#define PBR_TEXTURES_CONSTANT_H

// textures/constant.h*
#include "core/Texture.h"

namespace pbr
{

// ConstantTexture Declarations
template <typename T>
class ConstantTexture : public Texture<T>
{
public:
    // ConstantTexture Public Methods
    ConstantTexture(const T &value) : value(value) {}
    T Evaluate(const SurfaceInteraction &) const { return value; }

private:
    T value;
};

}  // namespace pbrt

#endif  // PBRT_TEXTURES_CONSTANT_H
