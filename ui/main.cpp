#include "Renderer.h"
#include "core/SceneLoader.h"
#include <cstdio>
#include <string>

namespace
{
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
        std::fprintf(report, "bounds: (%.3f, %.3f, %.3f) .. (%.3f, %.3f, %.3f)\n",
                     b.pMin.x, b.pMin.y, b.pMin.z, b.pMax.x, b.pMax.y, b.pMax.z);
        std::fprintf(report, "camera fov: %.1f near: %.2f far: %.2f\n",
                     loaded.camera.fov, loaded.camera.nearZ, loaded.camera.farZ);
        std::fprintf(report, "camera pos: (%.3f, %.3f, %.3f) target: (%.3f, %.3f, %.3f)\n",
                     loaded.camera.position.x, loaded.camera.position.y, loaded.camera.position.z,
                     loaded.camera.target.x, loaded.camera.target.y, loaded.camera.target.z);

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
        const char *scenePath = argc >= 3 ? argv[2] : "Resources/scenes/scene05_1.xml";
        const char *reportPath = argc >= 4 ? argv[3] : nullptr;
        return CheckScene(scenePath, reportPath);
    }

    Renderer renderer(800, 600);
    renderer.Run();

    return 0;
}
