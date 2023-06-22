
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
        // Compute differential changes in origin for orthographic camera rays
        dxCamera = RasterToCamera(Vector3f(1, 0, 0));
        dyCamera = RasterToCamera(Vector3f(0, 1, 0));
    }
    
    Float GenerateRay(const CameraSample &sample, Ray *ray) const;
    
    Float GenerateRayDifferential(const CameraSample &sample,
                                  RayDifferential *) const;
private:
    Vector3f dxCamera, dyCamera;
};

}  // namespace pbrt

#endif  // PBRT_CAMERAS_ORTHOGRAPHIC_H
