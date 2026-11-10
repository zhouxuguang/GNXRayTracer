#include "Renderer.h"
#include <MiniFB.h>
#include <cmath>
#include <cstdio>
#include <thread>
#include <vector>

#include "camera/Perspective.h"
#include "core/SceneLoader.h"

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
}

void Renderer::Run()
{
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

pbr::Spectrum Renderer::RenderPixel(int x, int y)
{
    pbr::Spectrum color(0.f);
    if (!mCamera)
        return color;

    // 工程相机使用连续胶片坐标，取像素中心；pLens / time 预留给景深与快门
    pbr::CameraSample sample;
    sample.pFilm = pbr::Point2f(x + 0.5f, y + 0.5f);
    sample.pLens = pbr::Point2f(0.f, 0.f);
    sample.time = 0.f;

    pbr::Ray ray;
    mCamera->GenerateRay(sample, &ray);

    color = GetIrradiance(ray);

    return color;

    // 场景未加载时回退到相机射线方向可视化，避免黑屏与空指针崩溃
    if (!mScene)
    {
        color[0] = pbr::Clamp(ray.d.x * 0.5f + 0.5f, 0.f, 1.f);
        color[1] = pbr::Clamp(ray.d.y * 0.5f + 0.5f, 0.f, 1.f);
        color[2] = pbr::Clamp(ray.d.z * 0.5f + 0.5f, 0.f, 1.f);
        return color;
    }

    pbr::SurfaceInteraction isect;
    if (!mScene->Intersect(ray, &isect))
        return color;

    // 世界空间法线从 [-1, 1] 映射到 [0, 1] 显示；未命中保持黑色背景
    color[0] = pbr::Clamp(isect.n.x * 0.5f + 0.5f, 0.f, 1.f);
    color[1] = pbr::Clamp(isect.n.y * 0.5f + 0.5f, 0.f, 1.f);
    color[2] = pbr::Clamp(isect.n.z * 0.5f + 0.5f, 0.f, 1.f);
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

pbr::Spectrum Renderer::GetIrradiance(const pbr::Ray& ray)
{
    pbr::Spectrum E = pbr::Spectrum(0.f);

    pbr::SurfaceInteraction isect;
    if (!mScene->Intersect(ray, &isect))
    {
        return E;
    }

    // 遍历各个光源，计算L
    for (auto light : mScene->lights)
    {
        pbr::Point2f u;
        pbr::Vector3f wi;
        pbr::Float pdf;
        pbr::VisibilityTester visibility;
        pbr::Spectrum L = light->Sample_Li(isect, u, &wi, &pdf, &visibility);

        // 生成阴影射线，判断当前着色点是否在阴影中
        if (!visibility.Unoccluded(*mScene))
        {
            continue;
        }

        pbr::Float cosTheta = pbr::Dot(isect.shading.n, wi);

        E += L * std::max(cosTheta, 0.0f);
    }
    return E;
}
