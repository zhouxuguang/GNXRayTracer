#ifndef GNXRAYTRACER_RENDERER_H
#define GNXRAYTRACER_RENDERER_H

#include <atomic>
#include <memory>
#include "core/Camera.h"
#include "core/Spectrum.h"
#include "core/Scene.h"

class Renderer
{
public:
    Renderer(int w, int h, const char* filePath);

    virtual ~Renderer()
    {
    }
    void Run();

    pbr::Spectrum RenderPixel(int x, int y);

private:
    void RunRenderThread();

    pbr::Spectrum GetIrradiance(const pbr::Ray& ray);

    int mViewportWidth = 800;
    int mViewportHeight = 600;
    uint32_t *mBuffer = nullptr;

    std::atomic_int mCurrentPixelIndex{0};

    // AnimatedTransform 内部只保存 Transform 的指针，需保证它比 mCamera 活得久
    pbr::Transform mCameraToWorld;
    std::shared_ptr<const pbr::Camera> mCamera;

    std::shared_ptr<pbr::Scene> mScene = nullptr;
    std::shared_ptr<pbr::Sampler> mSampler;
    std::shared_ptr<pbr::Integrator> mIntegrator = nullptr;
};

#endif // GNXRAYTRACER_RENDERER_H
