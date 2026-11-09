#ifndef PBR_CORE_SCENE_LOADER_H
#define PBR_CORE_SCENE_LOADER_H

#include "Scene.h"
#include <cstddef>
#include <memory>
#include <string>

namespace pbr
{

// <Camera> 节点解析结果。Scene 本身不持有相机，由调用方用它构造相机。
struct SceneCameraDesc
{
    Point3f position = Point3f(0.f, 0.f, 0.f);
    Point3f target = Point3f(0.f, 0.f, 1.f);
    Vector3f up = Vector3f(0.f, 1.f, 0.f);
    Float nearZ = 0.1f;
    Float farZ = 1000.f;
    Float fov = 60.f;
};

struct LoadedScene
{
    std::shared_ptr<Scene> scene;
    SceneCameraDesc camera;
    std::size_t objectCount = 0;
    std::size_t primitiveCount = 0;
};

// 从 XML 文件构建 Scene：SceneObject -> Primitive -> BVHAccel -> Scene。
// 失败时返回 false，并通过 error 返回原因。
bool LoadSceneFromXml(const std::string &filePath, LoadedScene &out,
                      std::string *error = nullptr);

} // namespace pbr

#endif // PBR_CORE_SCENE_LOADER_H
