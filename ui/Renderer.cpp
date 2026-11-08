#include "Renderer.h"
#include <MiniFB.h>
#include <thread>
#include <vector>

Renderer::Renderer(int w, int h)
	: mViewportWidth(w), 
	mViewportHeight(h)
{
	mCurrentPixelIndex = 0;
}

void Renderer::Run()
{
	struct mfb_window* window = mfb_open_ex("GNXRaytracer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
	if (window == NULL)
		return;

	// 屏幕/窗口/视口上每个像素点的颜色，以32位整数表示，格式为0xAARRGGBB（Alpha, Red, Green, Blue）
	mBuffer = (uint32_t*)malloc(mViewportWidth * mViewportHeight * 4);

	int numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> renderThreads(numThreads);
	for (int i = 0; i < numThreads; i++) 
	{
		renderThreads[i] = std::thread(&Renderer::RunRenderThread, this);
		renderThreads[i].detach();
	}

	// Present：
	mfb_update_state state;
	do 
    {
		state = mfb_update_ex(window, mBuffer, mViewportWidth, mViewportHeight);

		if (state != MFB_STATE_OK)
			break;

	} while (mfb_wait_sync(window));

	free(mBuffer);
	mBuffer = NULL;
	window = NULL;
}

pbr::Spectrum Renderer::RenderPixel(int x, int y)
{
	pbr::Spectrum color;
	color[0] = (float)x / mViewportWidth;
	color[1] = (float)y / mViewportHeight;
	color[2] = 0.0f;

	//std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 模拟渲染每个像素点需要的时间

	return color;

}

// 渲染线程的入口函数，负责执行渲染循环
void Renderer::RunRenderThread()
{
	while (true)
	{
        // 读取当前屏幕的下一个像素
        int pixelIndex = mCurrentPixelIndex.fetch_add(1);
        if (pixelIndex >= mViewportWidth * mViewportHeight)
            break;

        int x = pixelIndex % mViewportWidth;
        int y = pixelIndex / mViewportWidth;

        pbr::Spectrum color = RenderPixel(x, y);
        uint32_t r = std::round(color[0] * 255.0f);
        uint32_t g = std::round(color[1] * 255.0f);
        uint32_t b = std::round(color[2] * 255.0f);
        mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | (b);
	}
}
