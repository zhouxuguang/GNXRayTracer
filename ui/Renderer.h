#ifndef GNXRAYTRACER_RENDERER_H
#define GNXRAYTRACER_RENDERER_H

#include <atomic>
#include <memory>
#include "core/Camera.h"
#include "core/Spectrum.h"

class Renderer
{
public:
	Renderer(int w, int h);
	virtual ~Renderer() {}
	void Run();

private:
	pbr::Spectrum RenderPixel(int x, int y);
	void RunRenderThread();

	int mViewportWidth = 800;
	int mViewportHeight = 600;
	uint32_t* mBuffer = nullptr;

	std::atomic_int mCurrentPixelIndex{0};

	// AnimatedTransform 内部只保存 Transform 的指针，需保证它比 mCamera 活得久
	pbr::Transform mCameraToWorld;
	std::shared_ptr<const pbr::Camera> mCamera;
};

#endif // GNXRAYTRACER_RENDERER_H
