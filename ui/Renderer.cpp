#include "Renderer.h"
#include <MiniFB.h>
#include <cmath>
#include <cstdio>
#include <thread>
#include <vector>

#include "camera/Perspective.h"
#include "core/SceneLoader.h"
#include "core/Integrator.h"
#include "integrators/DirectLightingIntegrator.h"
#include "integrators/PathIntegrator.h"
#include "samplers/HaltonSampler.h"

Renderer::Renderer(int w, int h, const char *filePath)
    : mViewportWidth(w),
      mViewportHeight(h)
{
    mCurrentPixelIndex = 0;

    pbr::LoadedScene loaded;
    std::string error;
    const char *path = filePath ? filePath : "";
    if (!pbr::LoadSceneFromXml(path, loaded, &error) || !loaded.scene)
    {
        std::fprintf(stderr, "Failed to load scene '%s': %s\n", path, error.c_str());
    }
    mScene = loaded.scene;

    // 使用 XML 中描述的相机：位置、朝向、视场角
    const pbr::SceneCameraDesc &cam = loaded.camera;
    pbr::Transform worldToCamera = pbr::LookAt(cam.position, cam.target, cam.up);
    mCameraToWorld = Inverse(worldToCamera);
    pbr::AnimatedTransform cameraMotion(&mCameraToWorld, 0.f, &mCameraToWorld, 1.f);

    mCamera = std::shared_ptr<const pbr::Camera>(
        pbr::CreatePerspectiveCamera(mViewportWidth, mViewportHeight, cameraMotion, cam.fov));

    // 确定性采样器：每像素 1000 个样本，且固定只采像素中心 (x + 0.5, y + 0.5)
    const pbr::Bounds2i sampleBounds(
        pbr::Point2i(0, 0), pbr::Point2i(mViewportWidth, mViewportHeight));
    mSampler = std::shared_ptr<pbr::Sampler>(
        new pbr::HaltonSampler(1000, sampleBounds, true));

    // 直接光照积分器：相机与像素坐标在调用 Render 时传入，积分器只持有采样器
    mIntegrator = std::make_shared<pbr::PathIntegrator>(15, mSampler, 0.8);
}

void Renderer::Run()
{
    // 积分器预处理
	if (mIntegrator)
	{
		mIntegrator->Preprocess(*mScene);
	}

    struct mfb_window *window = mfb_open_ex("GNXRaytracer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
    if (window == NULL)
        return;

    // 屏幕/窗口/视口上每个像素点的颜色，以32位整数表示，格式为0xAARRGGBB（Alpha, Red, Green, Blue）
    mBuffer = (uint32_t *)malloc(mViewportWidth * mViewportHeight * 4);

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

pbr::Spectrum Renderer::RenderPixel(int x, int y, pbr::MemoryArena &arena)
{
    pbr::Spectrum color(0.f);
    if (!mIntegrator || !mScene || !mCamera)
        return color;

    // 主光线生成、spp 循环与采样器管理都交给积分器，结果通过回调拿回
    double timeConsume = 0.0;
    mIntegrator->Render(*mScene, *mCamera, x, y, arena, timeConsume,
                        [&color](int, int, const pbr::Spectrum &L)
                        {
                            color = L;
                        });

    return color;
}

pbr::Spectrum Renderer::RenderPixel(int x, int y)
{
    pbr::MemoryArena arena;
    return RenderPixel(x, y, arena);
}

// 渲染线程的入口函数，负责执行渲染循环
void Renderer::RunRenderThread()
{
    // 每个线程一个内存池，跨像素复用；MemoryArena 非线程安全，不能共享
    pbr::MemoryArena arena;

    while (true)
    {
        // 读取当前屏幕的下一个像素
        int pixelIndex = mCurrentPixelIndex.fetch_add(1);
        if (pixelIndex >= mViewportWidth * mViewportHeight)
            break;

        int x = pixelIndex % mViewportWidth;
        int y = pixelIndex / mViewportWidth;

        pbr::Spectrum color = RenderPixel(x, y, arena);
        uint32_t r = (uint32_t)pbr::Clamp(std::round(color[0] * 255.0f), 0.f, 255.f);
        uint32_t g = (uint32_t)pbr::Clamp(std::round(color[1] * 255.0f), 0.f, 255.f);
        uint32_t b = (uint32_t)pbr::Clamp(std::round(color[2] * 255.0f), 0.f, 255.f);
        mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | (b);
    }
}
