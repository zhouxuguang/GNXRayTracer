#include "SceneLoader.h"
#include "thirdparty/tinyxml2/tinyxml2.h"
#include "accelerator/BVHAccel.h"
#include "materials/MatteMaterial.h"
#include "shape/Sphere.h"
#include "shape/Triangle.h"
#include "textures/ConstantTexture.h"
#include <cstdlib>
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

std::shared_ptr<Material> MakeDefaultMaterial()
{
    auto kd = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.8f));
    auto sigma = std::make_shared<ConstantTexture<Float>>(0.f);
    auto bumpMap = std::make_shared<ConstantTexture<Float>>(0.f);
    return std::make_shared<MatteMaterial>(kd, sigma, bumpMap);
}

bool AppendObject(XMLElement *object, const std::shared_ptr<Material> &material,
                  std::vector<std::shared_ptr<Primitive>> &prims, std::string &error)
{
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
    std::vector<int> indices;
    for (XMLElement *triangle = shapes->FirstChildElement("Triangle"); triangle;
         triangle = triangle->NextSiblingElement("Triangle"))
    {
        Point3f v[3];
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
            v[count++] = Point3f(values[0], values[1], values[2]);
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
            indices.push_back(base + i);
        }
    }

    int nTriangles = static_cast<int>(indices.size() / 3);
    if (nTriangles > 0)
    {
        auto mesh = std::make_shared<TriangleMesh>(
            Identity(), nTriangles, indices.data(), static_cast<int>(vertices.size()),
            vertices.data(), nullptr, nullptr, nullptr, nullptr);
        for (int i = 0; i < nTriangles; ++i)
        {
            auto shape = std::make_shared<Triangle>(&Identity(), &Identity(), false, mesh, i);
            prims.push_back(std::make_shared<GeometricPrimitive>(
                shape, material, nullptr, MediumInterface()));
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
        prims.push_back(std::make_shared<GeometricPrimitive>(
            shape, material, nullptr, MediumInterface()));
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

    auto material = MakeDefaultMaterial();
    std::vector<std::shared_ptr<Primitive>> prims;
    std::size_t objectCount = 0;
    for (XMLElement *object = objects->FirstChildElement("SceneObject"); object;
         object = object->NextSiblingElement("SceneObject"))
    {
        if (!AppendObject(object, material, prims, detail))
            return fail("<SceneObject> #" + std::to_string(objectCount + 1) + ": " + detail);
        ++objectCount;
    }

    if (prims.empty())
        return fail("scene contains no primitives: " + filePath);

    std::vector<std::shared_ptr<Light>> lights;
    auto aggregate = std::make_shared<BVHAccel>(prims, 1, BVHAccel::SplitMethod::SAH);
    out.scene = std::make_shared<Scene>(aggregate, lights);
    out.objectCount = objectCount;
    out.primitiveCount = prims.size();
    return true;
}

} // namespace pbr
