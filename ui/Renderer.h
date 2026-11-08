#ifndef GNXRAYTRACER_RENDERER_H
#define GNXRAYTRACER_RENDERER_H

#include <atomic>
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

	std::atomic_int mCurrentPixelIndex = 0;
};

#endif // GNXRAYTRACER_RENDERER_H
