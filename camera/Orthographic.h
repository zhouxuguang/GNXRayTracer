
#ifndef PBR_CAMERAS_ORTHOGRAPHIC_H
#define PBR_CAMERAS_ORTHOGRAPHIC_H

// cameras/orthographic.h*
#include "core/Camera.h"

namespace pbr
{

class OrthographicCamera : public ProjectiveCamera
{
  public:
    // OrthographicCamera Public Methods
    OrthographicCamera(const int RasterWidth, const int RasterHeight, const Transform &CameraToWorld,
                       const Bounds2f &screenWindow, float lensRadius,
                       float focalDistance)
        : ProjectiveCamera(RasterWidth, RasterHeight, CameraToWorld, Orthographic(0, 10), screenWindow,
                        0, 0, lensRadius, focalDistance, nullptr, nullptr)
    {
    }
    
    float GenerateRay(const CameraSample &sample, Ray *ray) const;
};

}  // namespace pbrt

#endif  // PBRT_CAMERAS_ORTHOGRAPHIC_H
