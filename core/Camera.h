#ifndef PBR_CORE_CAMERA_H
#define PBR_CORE_CAMERA_H

// core/camera.h*
#include "Geometry.h"
#include "Transform.h"
#include "Spectrum.h"

namespace pbr
{

// Camera Declarations
class Camera
{
public:
    // Camera Interface
    Camera(const Transform &CameraToWorld, Float shutterOpen,
           Float shutterClose, Film *film, const Medium *medium);
    virtual ~Camera();
    virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const = 0;
    virtual Float GenerateRayDifferential(const CameraSample &sample, RayDifferential *rd) const;
    
    virtual Spectrum We(const Ray &ray, Point2f *pRaster2 = nullptr) const;
    virtual void Pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
    virtual Spectrum Sample_Wi(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf, Point2f *pRaster,
                               VisibilityTester *vis) const;

    // Camera Public Data
    Transform CameraToWorld;
    const Float shutterOpen, shutterClose;
    // Film *film;
    const Medium *medium;
};

struct CameraSample
{
    Point2f pFilm;
    Point2f pLens;
    Float time;
};

inline std::ostream &operator<<(std::ostream &os, const CameraSample &cs)
{
    os << "[ pFilm: " << cs.pFilm << " , pLens: " << cs.pLens <<
        StringPrintf(", time %f ]", cs.time);
    return os;
}

class ProjectiveCamera : public Camera
{
  public:
    // ProjectiveCamera Public Methods
    ProjectiveCamera(const int RasterWidth, const int RasterHeight, const Transform &CameraToWorld,
                     const Transform &CameraToScreen,
                     const Bounds2f &screenWindow, Float shutterOpen,
                     Float shutterClose, Float lensr, Float focald, Film *film,
                     const Medium *medium)
        : Camera(CameraToWorld, shutterOpen, shutterClose, film, medium),
          CameraToScreen(CameraToScreen) {
        // Initialize depth of field parameters
        lensRadius = lensr;
        focalDistance = focald;

        // Compute projective camera transformations

        // Compute projective camera screen transformations
        ScreenToRaster =
            Scale(RasterWidth, RasterHeight, 1) *
            Scale(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
                  1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
            Translate(Vector3f(-screenWindow.pMin.x, -screenWindow.pMax.y, 0));
        RasterToScreen = Inverse(ScreenToRaster);
        RasterToCamera = Inverse(CameraToScreen) * RasterToScreen;
    }

protected:
    // ProjectiveCamera Protected Data
    Transform CameraToScreen, RasterToCamera;
    Transform ScreenToRaster, RasterToScreen;
    Float lensRadius, focalDistance;
};

class PerspectiveCamera;
class OrthographicCamera;

PerspectiveCamera *CreatePerspectiveCamera(const int RasterWidth, const int RasterHeight, const Transform &cam2world);
OrthographicCamera *CreateOrthographicCamera(const int RasterWidth, const int RasterHeight, const Transform &cam2world);

}  // namespace pbr

#endif  // PBR_CORE_CAMERA_H
