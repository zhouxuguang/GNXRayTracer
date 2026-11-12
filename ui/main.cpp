#include "Renderer.h"
#include "core/SceneLoader.h"
#include <cstdio>
#include <string>

namespace
{
bool FileExists(const char *path)
{
    FILE *f = std::fopen(path, "r");
    if (!f)
        return false;
    std::fclose(f);
    return true;
}

std::string ResolveScenePath(int argc, char *argv[])
{
    for (int i = 1; i + 1 < argc; ++i)
    {
        if (std::string(argv[i]) == "--scene")
            return argv[i + 1];
    }

    const char *candidates[] = {
        "Resources/scenes/scene09.xml",
        "../Resources/scenes/scene09.xml",
        "../../Resources/scenes/scene09.xml",
    };
    for (const char *candidate : candidates)
    {
        if (FileExists(candidate))
            return candidate;
    }
    return candidates[0];
}

int CheckScene(const char *scenePath, const char *reportPath)
{
    FILE *report = reportPath ? std::fopen(reportPath, "w") : stderr;
    if (!report)
        return 2;

    pbr::LoadedScene loaded;
    std::string error;
    bool ok = pbr::LoadSceneFromXml(scenePath, loaded, &error);

    std::fprintf(report, "scene: %s\n", scenePath);
    std::fprintf(report, "load: %s\n", ok ? "OK" : "FAILED");
    if (!ok)
    {
        std::fprintf(report, "error: %s\n", error.c_str());
    }
    else
    {
        const pbr::Bounds3f &b = loaded.scene->WorldBound();
        std::fprintf(report, "objects: %zu\n", loaded.objectCount);
        std::fprintf(report, "primitives: %zu\n", loaded.primitiveCount);
        std::fprintf(report, "lights: %zu\n", loaded.lightCount);
        std::fprintf(report, "bounds: (%.3f, %.3f, %.3f) .. (%.3f, %.3f, %.3f)\n",
                     b.pMin.x, b.pMin.y, b.pMin.z, b.pMax.x, b.pMax.y, b.pMax.z);
        std::fprintf(report, "camera fov: %.1f near: %.2f far: %.2f\n",
                     loaded.camera.fov, loaded.camera.nearZ, loaded.camera.farZ);
        std::fprintf(report, "camera pos: (%.3f, %.3f, %.3f) target: (%.3f, %.3f, %.3f)\n",
                     loaded.camera.position.x, loaded.camera.position.y, loaded.camera.position.z,
                     loaded.camera.target.x, loaded.camera.target.y, loaded.camera.target.z);

        // 逐个调用 Sample_Li，验证灯光确实进入 Scene 且可用
        pbr::Interaction ref;
        ref.p = pbr::Point3f(0.f, 0.75f, 0.f);
        for (std::size_t i = 0; i < loaded.scene->lights.size(); ++i)
        {
            pbr::Vector3f wi;
            pbr::Float pdf = 0.f;
            pbr::VisibilityTester vis;
            pbr::Spectrum radiance = loaded.scene->lights[i]->Sample_Li(
                ref, pbr::Point2f(0.f, 0.f), &wi, &pdf, &vis);
            std::fprintf(report, "light[%zu] flags=%d L=(%.4f, %.4f, %.4f)\n",
                         i, loaded.scene->lights[i]->flags,
                         radiance[0], radiance[1], radiance[2]);
        }

        pbr::Vector3f dir = pbr::Normalize(loaded.camera.target - loaded.camera.position);
        pbr::Ray primary(loaded.camera.position, dir);
        pbr::SurfaceInteraction hitPoint;
        bool hit = loaded.scene->Intersect(primary, &hitPoint);
        std::fprintf(report, "primary ray hit: %d", hit ? 1 : 0);
        if (hit)
            std::fprintf(report, " at (%.3f, %.3f, %.3f)", hitPoint.p.x, hitPoint.p.y, hitPoint.p.z);
        std::fprintf(report, "\n");

        pbr::Ray backward(loaded.camera.position, -dir);
        pbr::SurfaceInteraction unused;
        std::fprintf(report, "backward ray hit: %d\n",
                     loaded.scene->Intersect(backward, &unused) ? 1 : 0);

        // 验证 RenderPixel 的法线着色输出（构造 Renderer 不会打开窗口）
        Renderer probe(800, 600, scenePath);
        pbr::Spectrum center = probe.RenderPixel(400, 300);
        pbr::Spectrum corner = probe.RenderPixel(5, 5);
        std::fprintf(report, "center pixel normal rgb: (%.3f, %.3f, %.3f)\n",
                     center[0], center[1], center[2]);
        std::fprintf(report, "corner pixel rgb: (%.3f, %.3f, %.3f)\n",
                     corner[0], corner[1], corner[2]);
    }

    if (reportPath)
        std::fclose(report);
    return ok ? 0 : 1;
}
} // namespace

int main(int argc, char *argv[])
{
    if (argc >= 2 && std::string(argv[1]) == "--check-scene")
    {
        const char *scenePath = argc >= 3 ? argv[2] : "Resources/scenes/scene09.xml";
        const char *reportPath = argc >= 4 ? argv[3] : nullptr;
        return CheckScene(scenePath, reportPath);
    }

    std::string scenePath = ResolveScenePath(argc, argv);
    Renderer renderer(800, 600, scenePath.c_str());
    renderer.Run();

    return 0;
}
