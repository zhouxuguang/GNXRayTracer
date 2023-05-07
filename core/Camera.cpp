
// core/camera.cpp*
#include "Camera.h"
//#include "sampling.h"
//#include "sampler.h"

namespace pbr
{

// Camera Method Definitions
Camera::~Camera()
{
//    delete film;
}

Camera::Camera(const Transform &CameraToWorld, Float shutterOpen,
               Float shutterClose, Film *film, const Medium *medium)
    : CameraToWorld(CameraToWorld)
//      shutterOpen(shutterOpen),
//      shutterClose(shutterClose),
//      film(film),
//      medium(medium)
{
//    if (CameraToWorld.HasScale())
//        Warning(
//            "Scaling detected in world-to-camera transformation!\n"
//            "The system has numerous assumptions, implicit and explicit,\n"
//            "that this transform will have no scale factors in it.\n"
//            "Proceed at your own risk; your image may have errors or\n"
//            "the system may crash as a result of this.");
}

Spectrum Camera::We(const Ray &ray, Point2f *raster) const
{
    //LOG(FATAL) << "Camera::We() is not implemented!";
    return Spectrum(0.f);
}

void Camera::Pdf_We(const Ray &ray, Float *pdfPos, Float *pdfDir) const
{
    //LOG(FATAL) << "Camera::Pdf_We() is not implemented!";
}

Spectrum Camera::Sample_Wi(const Interaction &ref, const Point2f &u,
                           Vector3f *wi, Float *pdf, Point2f *pRaster,
                           VisibilityTester *vis) const {
    //LOG(FATAL) << "Camera::Sample_Wi() is not implemented!";
    return Spectrum(0.f);
}

}  // namespace pbr
