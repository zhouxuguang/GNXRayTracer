
#ifndef PBR_CAMERAS_PERSPECTIVE_H
#define PBR_CAMERAS_PERSPECTIVE_H

// cameras/perspective.h*
#include "core/Camera.h"

namespace pbr
{

// PerspectiveCamera Declarations
class PerspectiveCamera : public ProjectiveCamera
{
  public:
    // PerspectiveCamera Public Methods
    PerspectiveCamera(const int rasterWidth, const int rasterHeight, const Transform &CameraToWorld,
                      const Bounds2f &screenWindow, float lensRadius, float focalDistance, float fov);
    
    Float GenerateRay(const CameraSample &sample, Ray *) const;
    
    Float GenerateRayDifferential(const CameraSample &sample, RayDifferential *ray) const;
    
private:
    Vector3f dxCamera, dyCamera;
};

//PerspectiveCamera *CreatePerspectiveCamera(const ParamSet &params,
//                                           const AnimatedTransform &cam2world,
//                                           Film *film, const Medium *medium);

}  // namespace pbrt

#endif  // PBRT_CAMERAS_PERSPECTIVE_H
