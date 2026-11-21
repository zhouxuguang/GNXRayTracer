#include "SceneLoader.h"
#include "core/Texture.h"
#include "thirdparty/tinyxml2/tinyxml2.h"
#include "accelerator/BVHAccel.h"
#include "lights/DiffuseAreaLight.h"
#include "lights/DistantLight.h"
#include "lights/PointLight.h"
#include "lights/SpotLight.h"
#include "materials/GlassMaterial.h"
#include "materials/MatteMaterial.h"
#include "materials/MirrorMaterial.h"
#include "shape/Sphere.h"
#include "shape/Triangle.h"
#include "textures/ConstantTexture.h"
#include "textures/ImageTexture.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>

using namespace tinyxml2;

namespace pbr
{

namespace
{
const Transform &Identity()
{
    static const Transform identity;
    return identity;
}

bool ParseNumbers(const char *text, std::vector<Float> &values)
{
    values.clear();
    if (!text)
        return false;
    const char *p = text;
    while (*p)
    {
        while (*p == ',' || *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            ++p;
        if (!*p)
            break;
        char *end = nullptr;
        Float value = std::strtof(p, &end);
        if (end == p)
            return false;
        values.push_back(value);
        p = end;
    }
    return !values.empty();
}

const char *ChildText(const XMLElement *parent, const char *name)
{
    if (!parent)
        return nullptr;
    const XMLElement *child = parent->FirstChildElement(name);
    return child ? child->GetText() : nullptr;
}

bool ReadTriple(const XMLElement *parent, const char *name, Vector3f &out,
                std::string &error, bool required = true)
{
    const char *text = ChildText(parent, name);
    if (!text)
    {
        if (required)
            error = std::string("missing <") + name + ">";
        return !required;
    }
    std::vector<Float> values;
    if (!ParseNumbers(text, values) || values.size() < 3)
    {
        error = std::string("invalid <") + name + ">: " + text;
        return false;
    }
    out = Vector3f(values[0], values[1], values[2]);
    return true;
}

bool ReadScalar(const XMLElement *parent, const char *name, Float &out,
                std::string &error, bool required = true)
{
    const char *text = ChildText(parent, name);
    if (!text)
    {
        if (required)
            error = std::string("missing <") + name + ">";
        return !required;
    }
    std::vector<Float> values;
    if (!ParseNumbers(text, values))
    {
        error = std::string("invalid <") + name + ">: " + text;
        return false;
    }
    out = values[0];
    return true;
}

bool ReadRgb(const XMLElement *parent, const char *name, Spectrum &out,
             std::string &error, bool required = true)
{
    const char *text = ChildText(parent, name);
    if (!text)
    {
        if (required)
            error = std::string("missing <") + name + ">";
        return !required;
    }
    std::vector<Float> values;
    if (!ParseNumbers(text, values) || values.size() < 3)
    {
        error = std::string("invalid <") + name + ">: " + text;
        return false;
    }
    out = Spectrum(values[0]);
    out[0] = values[0];
    out[1] = values[1];
    out[2] = values[2];
    return true;
}

// <Attenuations> 的三个数按 (A 二次项, B 一次项, C 常数项) 读取，
// 与 DistanceAttenuation 的 1/(C + B*d + A*d^2) 约定一致；缺省为 (1,0,0)。
bool ReadAttenuations(const XMLElement *parent, Vector3f &out, std::string &error)
{
    out = Vector3f(1.f, 0.f, 0.f);
    const char *text = ChildText(parent, "Attenuations");
    if (!text)
        return true;
    std::vector<Float> values;
    if (!ParseNumbers(text, values) || values.size() < 3)
    {
        error = std::string("invalid <Attenuations>: ") + text;
        return false;
    }
    out = Vector3f(values[0], values[1], values[2]);
    return true;
}

// 聚光灯在局部空间的 +Z 是光锥主轴；LookAt 返回 world->light，取逆后
// 正好把局部 +Z 映射到 <Direction>，同时把原点平移到 <Position>。
bool MakeSpotTransform(const Point3f &position, const Vector3f &direction,
                       Transform &out, std::string &error)
{
    if (direction.LengthSquared() <= 0)
    {
        error = "<Direction> must be non-zero";
        return false;
    }
    Vector3f dir = Normalize(direction);
    Vector3f up(0.f, 1.f, 0.f);
    if (std::abs(dir.y) > 0.999f)
        up = Vector3f(1.f, 0.f, 0.f);
    out = Inverse(LookAt(position, position + dir, up));
    return true;
}

bool ReadTransform(const XMLElement *node, Transform &out, std::string &error)
{
    out = Transform();
    if (!node)
        return true;

    Vector3f position(0.f, 0.f, 0.f);
    Vector3f rotation(0.f, 0.f, 0.f);
    Vector3f scale(1.f, 1.f, 1.f);
    if (!ReadTriple(node, "Position", position, error, false))
        return false;
    if (!ReadTriple(node, "Rotation", rotation, error, false))
        return false;

    const char *scaleText = ChildText(node, "Scale");
    if (scaleText)
    {
        std::vector<Float> values;
        if (!ParseNumbers(scaleText, values))
        {
            error = std::string("invalid <Scale>: ") + scaleText;
            return false;
        }
        if (values.size() == 1)
            scale = Vector3f(values[0], values[0], values[0]);
        else if (values.size() >= 3)
            scale = Vector3f(values[0], values[1], values[2]);
        else
        {
            error = std::string("invalid <Scale>: ") + scaleText;
            return false;
        }
    }

    out = Translate(position) * RotateZ(rotation.z) * RotateY(rotation.y) *
          RotateX(rotation.x) * Scale(scale.x, scale.y, scale.z);
    return true;
}

std::string TrimText(const char *text)
{
    if (!text)
        return std::string();
    std::string value(text);
    const char *whitespace = " \t\r\n";
    std::size_t begin = value.find_first_not_of(whitespace);
    if (begin == std::string::npos)
        return std::string();
    std::size_t end = value.find_last_not_of(whitespace);
    return value.substr(begin, end - begin + 1);
}

// 材质贴图统一放在场景文件同级的 texture/ 子目录下
std::string SceneTextureDir(const std::string &scenePath)
{
    const std::size_t slash = scenePath.find_last_of("/\\");
    const std::string dir =
        slash == std::string::npos ? std::string() : scenePath.substr(0, slash + 1);
    return dir + "texture/";
}

bool FileExists(const std::string &path)
{
    FILE *file = std::fopen(path.c_str(), "rb");
    if (!file)
        return false;
    std::fclose(file);
    return true;
}

// 读取 <AlbedoMap>：贴图存在就用贴图（丢弃 XML 里的数值），
// 缺 <AlbedoMap> 或文件不存在则回退到 <Albedo> 常量。
std::shared_ptr<Texture<Spectrum>> MakeAlbedoTexture(const XMLElement *node,
                                                     const std::string &materialName,
                                                     const std::string &textureDir,
                                                     std::string &error)
{
    const std::string albedoMap = TrimText(ChildText(node, "AlbedoMap"));
    if (!albedoMap.empty())
    {
        const std::string path = textureDir + albedoMap;
        if (FileExists(path))
        {
            // 反照率贴图按 sRGB 解码，三线性过滤 + 8x 各向异性
            return std::make_shared<ImageTexture<RGBSpectrum, Spectrum>>(
                std::make_unique<UVMapping2D>(), path, true, 8.f,
                ImageWrap::Repeat, 1.f, true);
        }
        std::fprintf(stderr, "warning: material '%s': albedo map not found: %s\n",
                     materialName.c_str(), path.c_str());
    }

    Spectrum albedo;
    if (!ReadRgb(node, "Albedo", albedo, error))
    {
        error = "material '" + materialName + "': " + error;
        return nullptr;
    }
    return std::make_shared<ConstantTexture<Spectrum>>(albedo);
}

using MaterialMap = std::map<std::string, std::shared_ptr<Material>>;

std::shared_ptr<Material> MakeDefaultMaterial()
{
    auto kd = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.8f));
    auto sigma = std::make_shared<ConstantTexture<Float>>(0.f);
    auto bumpMap = std::make_shared<ConstantTexture<Float>>(0.f);
    return std::make_shared<MatteMaterial>(kd, sigma, bumpMap);
}

// 解析 <Materials> 下的具名材质，供 <SceneObject> 的 <Material> 按名称引用。
bool AppendMaterials(XMLElement *materialNodes, const std::string &textureDir,
                     MaterialMap &materials, std::string &error)
{
    for (XMLElement *node = materialNodes->FirstChildElement("Material"); node;
         node = node->NextSiblingElement("Material"))
    {
        const std::string name = TrimText(ChildText(node, "Name"));
        if (name.empty())
        {
            error = "<Material> requires a non-empty <Name>";
            return false;
        }
        if (materials.find(name) != materials.end())
        {
            error = "duplicate material name: " + name;
            return false;
        }

        const std::string type = TrimText(ChildText(node, "Type"));
        if (type.empty())
        {
            error = "material '" + name + "' requires a <Type>";
            return false;
        }

        // Lambert 与 OrenNayar 共用 MatteMaterial：<Sigma> 为 0 时退化为 Lambertian，
        // 大于 0 时由 MatteMaterial 换成 Oren-Nayar 粗糙漫反射
        if (type == "Lambert" || type == "OrenNayar")
        {
            std::shared_ptr<Texture<Spectrum>> kd =
                MakeAlbedoTexture(node, name, textureDir, error);
            if (!kd)
                return false;
            // <Sigma> 是粗糙度（角度制），缺省 0 表示理想漫反射
            Float sigma = 0.f;
            if (type == "OrenNayar" &&
                !ReadScalar(node, "Sigma", sigma, error, false))
            {
                error = "material '" + name + "': " + error;
                return false;
            }
            if (sigma < 0.f || sigma > 90.f)
            {
                error = "material '" + name + "': <Sigma> must be within [0, 90] degrees";
                return false;
            }
            materials[name] = std::make_shared<MatteMaterial>(
                kd,
                std::make_shared<ConstantTexture<Float>>(sigma),
                std::make_shared<ConstantTexture<Float>>(0.f));
        }
        else if (type == "Mirror" || type == "ConductorSpecular")
        {
            Spectrum reflectionColor(1.f);
            if (!ReadRgb(node, "ReflectionColor", reflectionColor, error, false))
            {
                error = "material '" + name + "': " + error;
                return false;
            }
            auto kr = std::make_shared<ConstantTexture<Spectrum>>(reflectionColor);

            if (type == "Mirror")
            {
                materials[name] = std::make_shared<MirrorMaterial>(kr, nullptr);
            }
            else
            {
                Spectrum eta, absorptionCoef;
                if (!ReadRgb(node, "Eta", eta, error) ||
                    !ReadRgb(node, "AbsorptionCoef", absorptionCoef, error))
                {
                    error = "material '" + name + "': " + error;
                    return false;
                }
                materials[name] = std::make_shared<MirrorMaterial>(
                    kr,
                    std::make_shared<ConstantTexture<Spectrum>>(eta),
                    std::make_shared<ConstantTexture<Spectrum>>(absorptionCoef),
                    nullptr);
            }
        }
        else if (type == "DielectricSpecular" || type == "Glass")
        {
            // 与教程 DielectricSpecularMaterial(eta, transmissionColor) 等价：
            // 反射无色、理想光滑、无 bump，外侧 IOR 固定为 1
            Float eta = 1.5f;
            if (!ReadScalar(node, "Eta", eta, error, false))
            {
                error = "material '" + name + "': " + error;
                return false;
            }
            // 教程 ParseVector3f(nullptr) 在字段缺失时返回 (0,0,0)
            Spectrum transmissionColor(0.f);
            if (!ReadRgb(node, "TransmissionColor", transmissionColor, error, false))
            {
                error = "material '" + name + "': " + error;
                return false;
            }
            materials[name] = std::make_shared<GlassMaterial>(
                // 教程的反射项只有 Fresnel 系数，等价于反射颜色恒为 (1,1,1)
                std::make_shared<ConstantTexture<Spectrum>>(Spectrum(1.f)),
                std::make_shared<ConstantTexture<Spectrum>>(transmissionColor),
                std::make_shared<ConstantTexture<Float>>(0.f),
                std::make_shared<ConstantTexture<Float>>(0.f),
                std::make_shared<ConstantTexture<Float>>(eta),
                nullptr,
                true);
        }
        else
        {
            error = "material '" + name + "' has unsupported <Type>: " + type;
            return false;
        }
    }
    return true;
}

bool AppendObject(XMLElement *object, const MaterialMap &materials,
                  const std::shared_ptr<Material> &defaultMaterial,
                  std::vector<std::shared_ptr<Primitive>> &prims,
                  std::vector<std::shared_ptr<Light>> &lights, std::string &error)
{
    std::shared_ptr<Material> material = defaultMaterial;
    const char *materialText = ChildText(object, "Material");
    if (materialText)
    {
        const std::string materialName = TrimText(materialText);
        if (materialName.empty())
        {
            error = "<Material> name is empty";
            return false;
        }
        auto it = materials.find(materialName);
        if (it == materials.end())
        {
            error = "material '" + materialName + "' is not defined in <Materials>";
            return false;
        }
        material = it->second;
    }

    // <Emissive> 让该对象的形状同时充当漫反射面光源，值为出射辐射亮度 Le
    Spectrum emissive(0.f);
    if (!ReadRgb(object, "Emissive", emissive, error, false))
        return false;

    Transform objectToWorld;
    if (!ReadTransform(object->FirstChildElement("Transform"), objectToWorld, error))
        return false;

    XMLElement *shapes = object->FirstChildElement("Primitives");
    if (!shapes)
    {
        error = "missing <Primitives>";
        return false;
    }

    std::vector<Point3f> vertices;
    std::vector<Point2f> uvs;
    std::vector<int> indices;
    // <Vertex> 写成 "x,y,z" 或 "x,y,z,u,v"；只有所有顶点都带 uv 时才启用纹理坐标
    bool hasUV = true;
    for (XMLElement *triangle = shapes->FirstChildElement("Triangle"); triangle;
         triangle = triangle->NextSiblingElement("Triangle"))
    {
        Point3f v[3];
        Point2f uv[3];
        int count = 0;
        for (XMLElement *vertex = triangle->FirstChildElement("Vertex");
             vertex && count < 3; vertex = vertex->NextSiblingElement("Vertex"))
        {
            std::vector<Float> values;
            if (!ParseNumbers(vertex->GetText(), values) || values.size() < 3)
            {
                error = "invalid <Vertex> in <Triangle>";
                return false;
            }
            v[count] = Point3f(values[0], values[1], values[2]);
            if (values.size() >= 5)
                uv[count] = Point2f(values[3], values[4]);
            else
                hasUV = false;
            ++count;
        }
        if (count != 3)
        {
            error = "<Triangle> requires exactly 3 <Vertex> elements";
            return false;
        }

        int base = static_cast<int>(vertices.size());
        for (int i = 0; i < 3; ++i)
        {
            vertices.push_back(objectToWorld(v[i]));
            uvs.push_back(uv[i]);
            indices.push_back(base + i);
        }
    }

    int nTriangles = static_cast<int>(indices.size() / 3);
    if (nTriangles > 0)
    {
        auto mesh = std::make_shared<TriangleMesh>(
            Identity(), nTriangles, indices.data(), static_cast<int>(vertices.size()),
            vertices.data(), nullptr, nullptr, hasUV ? uvs.data() : nullptr, nullptr);
        for (int i = 0; i < nTriangles; ++i)
        {
            auto shape = std::make_shared<Triangle>(&Identity(), &Identity(), false, mesh, i);
            std::shared_ptr<DiffuseAreaLight> areaLight;
            if (!emissive.IsBlack())
                areaLight = std::make_shared<DiffuseAreaLight>(
                    Identity(), MediumInterface(), emissive, 1, shape);
            prims.push_back(std::make_shared<GeometricPrimitive>(
                shape, material, areaLight, MediumInterface()));
            if (areaLight)
                lights.push_back(areaLight);
        }
    }

    for (XMLElement *sphere = shapes->FirstChildElement("Sphere"); sphere;
         sphere = sphere->NextSiblingElement("Sphere"))
    {
        Float radius = 0.f;
        if (!ReadScalar(sphere, "Radius", radius, error))
            return false;
        if (radius <= 0)
        {
            error = "<Sphere> radius must be positive";
            return false;
        }

        Point3f center = objectToWorld(Point3f(0.f, 0.f, 0.f));
        Float scaleFactor = objectToWorld(Vector3f(1.f, 0.f, 0.f)).Length();
        if (scaleFactor <= 0)
            scaleFactor = 1.f;
        auto shape = std::make_shared<Sphere>(radius * scaleFactor, center);
        std::shared_ptr<DiffuseAreaLight> areaLight;
        if (!emissive.IsBlack())
            areaLight = std::make_shared<DiffuseAreaLight>(
                Identity(), MediumInterface(), emissive, 1, shape);
        prims.push_back(std::make_shared<GeometricPrimitive>(
            shape, material, areaLight, MediumInterface()));
        if (areaLight)
            lights.push_back(areaLight);
    }
    return true;
}

bool AppendLights(XMLElement *lightNodes, std::vector<std::shared_ptr<Light>> &lights,
                  std::string &error)
{
    for (XMLElement *node = lightNodes->FirstChildElement("PointLight"); node;
         node = node->NextSiblingElement("PointLight"))
    {
        Vector3f position(0.f, 0.f, 0.f);
        Vector3f attenuation(1.f, 0.f, 0.f);
        Spectrum intensity;
        if (!ReadTriple(node, "Position", position, error))
            return false;
        if (!ReadRgb(node, "Intensity", intensity, error))
            return false;
        if (!ReadAttenuations(node, attenuation, error))
            return false;

        Transform lightToWorld = Translate(position);
        lights.push_back(std::make_shared<PointLight>(
            lightToWorld, MediumInterface(), intensity, attenuation));
    }

    for (XMLElement *node = lightNodes->FirstChildElement("SpotLight"); node;
         node = node->NextSiblingElement("SpotLight"))
    {
        Vector3f position(0.f, 0.f, 0.f);
        Vector3f direction(0.f, 0.f, 1.f);
        Vector3f attenuation(1.f, 0.f, 0.f);
        Spectrum intensity;
        Float innerAngle = 10.f;
        Float outerAngle = 45.f;
        if (!ReadTriple(node, "Position", position, error))
            return false;
        if (!ReadTriple(node, "Direction", direction, error))
            return false;
        if (!ReadRgb(node, "Intensity", intensity, error))
            return false;
        if (!ReadScalar(node, "InnerAngle", innerAngle, error, false))
            return false;
        if (!ReadScalar(node, "OuterAngle", outerAngle, error, false))
            return false;
        if (!ReadAttenuations(node, attenuation, error))
            return false;
        if (innerAngle <= 0 || outerAngle <= innerAngle || outerAngle >= 180)
        {
            error = "<SpotLight> requires 0 < InnerAngle < OuterAngle < 180";
            return false;
        }

        Transform lightToWorld;
        if (!MakeSpotTransform(Point3f(position.x, position.y, position.z),
                               direction, lightToWorld, error))
            return false;
        // SpotLight 的 totalWidth 是光锥外角，falloffStart 是开始衰减的内角
        lights.push_back(std::make_shared<SpotLight>(
            lightToWorld, MediumInterface(), intensity, outerAngle, innerAngle,
            attenuation));
    }

    for (XMLElement *node = lightNodes->FirstChildElement("DirectionalLight"); node;
         node = node->NextSiblingElement("DirectionalLight"))
    {
        Vector3f direction(0.f, 0.f, 1.f);
        Spectrum radiance;
        if (!ReadTriple(node, "Direction", direction, error))
            return false;
        if (!ReadRgb(node, "Radiance", radiance, error))
            return false;
        if (direction.LengthSquared() <= 0)
        {
            error = "<DirectionalLight> direction must be non-zero";
            return false;
        }
        // XML 的 <Direction> 是光线传播方向；Sample_Li 的 wi 指向光源，故取负
        Vector3f toLight = -Normalize(direction);
        lights.push_back(std::make_shared<DistantLight>(Identity(), radiance, toLight));
    }
    return true;
}
} // namespace

bool LoadSceneFromXml(const std::string &filePath, LoadedScene &out, std::string *error)
{
    auto fail = [error](const std::string &message) -> bool
    {
        if (error)
            *error = message;
        return false;
    };

    XMLDocument doc;
    if (doc.LoadFile(filePath.c_str()) != XML_SUCCESS)
        return fail("cannot open or parse XML: " + filePath);

    XMLElement *root = doc.FirstChildElement("Scene");
    if (!root)
        return fail("root <Scene> not found in " + filePath);

    std::string detail;
    XMLElement *camera = root->FirstChildElement("Camera");
    if (camera)
    {
        Vector3f position(0.f, 0.f, 0.f);
        Vector3f target(0.f, 0.f, 1.f);
        Vector3f up(0.f, 1.f, 0.f);
        if (!ReadTriple(camera, "Position", position, detail, false))
            return fail("Camera: " + detail);
        if (!ReadTriple(camera, "Target", target, detail, false))
            return fail("Camera: " + detail);
        if (!ReadTriple(camera, "Up", up, detail, false))
            return fail("Camera: " + detail);
        if (!ReadScalar(camera, "NearZ", out.camera.nearZ, detail, false))
            return fail("Camera: " + detail);
        if (!ReadScalar(camera, "FarZ", out.camera.farZ, detail, false))
            return fail("Camera: " + detail);
        if (!ReadScalar(camera, "Fov", out.camera.fov, detail, false))
            return fail("Camera: " + detail);

        out.camera.position = Point3f(position.x, position.y, position.z);
        out.camera.target = Point3f(target.x, target.y, target.z);
        out.camera.up = up;
    }

    XMLElement *objects = root->FirstChildElement("SceneObjects");
    if (!objects)
        return fail("<SceneObjects> not found in " + filePath);

    std::shared_ptr<Material> defaultMaterial = MakeDefaultMaterial();
    MaterialMap materials;
    const std::string textureDir = SceneTextureDir(filePath);
    XMLElement *materialNodes = root->FirstChildElement("Materials");
    if (materialNodes && !AppendMaterials(materialNodes, textureDir, materials, detail))
        return fail("<Materials>: " + detail);

    std::vector<std::shared_ptr<Primitive>> prims;
    std::vector<std::shared_ptr<Light>> lights;
    std::size_t objectCount = 0;
    for (XMLElement *object = objects->FirstChildElement("SceneObject"); object;
         object = object->NextSiblingElement("SceneObject"))
    {
        if (!AppendObject(object, materials, defaultMaterial, prims, lights, detail))
            return fail("<SceneObject> #" + std::to_string(objectCount + 1) + ": " + detail);
        ++objectCount;
    }

    if (prims.empty())
        return fail("scene contains no primitives: " + filePath);

    XMLElement *lightNodes = root->FirstChildElement("Lights");
    if (lightNodes && !AppendLights(lightNodes, lights, detail))
        return fail("<Lights>: " + detail);

    auto aggregate = std::make_shared<BVHAccel>(prims, 1, BVHAccel::SplitMethod::SAH);
    out.scene = std::make_shared<Scene>(aggregate, lights);
    out.objectCount = objectCount;
    out.primitiveCount = prims.size();
    out.lightCount = lights.size();
    return true;
}

} // namespace pbr
