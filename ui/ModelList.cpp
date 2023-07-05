//
//  ModelList.cpp
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/7/5.
//

#include "ModelList.h"
#include "DebugText.h"
#include "FrameBuffer.h"
#include "shape/plyRead.h"
#include "MaterialList.h"
#include "lights/DiffuseAreaLight.h"
#include "lights/SpotLight.h"
#include "lights/DistantLight.h"
#include "lights/SkyBoxLight.h"
#include "lights/InfiniteAreaLight.h"
#include "core/Sampling.h"

void AddFloor(std::vector<std::shared_ptr<Primitive>> &prims, std::shared_ptr<Material> material)
{
    //地板
    int nTrianglesFloor = 2;
    int vertexIndicesFloor[6] = { 0 ,1 ,2 ,3 ,4 ,5 };
    int nVerticesFloor = 6;
    const float yPos_Floor = -2.0;
    Point3f P_Floor[6] = {
        Point3f(-6.0, yPos_Floor, 6.0) ,Point3f(6.0, yPos_Floor, 6.0) , Point3f(-6.0, yPos_Floor, -6.0) ,
        Point3f(6.0, yPos_Floor, 6.0) ,Point3f(6.0, yPos_Floor, -6.0), Point3f(-6.0, yPos_Floor, -6.0)
    };
    
    Transform tri_Object2World2, tri_World2Object2;
    std::shared_ptr<TriangleMesh> meshFloor = std::make_shared<TriangleMesh>(tri_Object2World2, nTrianglesFloor, vertexIndicesFloor,
    nVerticesFloor, P_Floor, nullptr, nullptr, nullptr, nullptr);
    
    std::vector<std::shared_ptr<Shape>> trisFloor;
    for (int i = 0; i < nTrianglesFloor; ++i)
    {
        trisFloor.push_back(std::make_shared<Triangle>(&tri_Object2World2, &tri_World2Object2, false, meshFloor, i));
    }
    
    //将物体填充到基元
    for (int i = 0; i < nTrianglesFloor; ++i)
    {
        prims.push_back(std::make_shared<GeometricPrimitive>(trisFloor[i], material, nullptr, MediumInterface()));
    }
}

void AddModel(std::vector<std::shared_ptr<Primitive>> &prims, std::shared_ptr<Material> material)
{
    std::shared_ptr<TriangleMesh> mesh;
    std::vector<std::shared_ptr<Shape>> tris;

    Transform tri_Object2World, tri_World2Object;

    tri_Object2World = Translate(Vector3f(0.f, -2.9f, 0.f)) * tri_Object2World;
    tri_World2Object = Inverse(tri_Object2World);

    plyInfo plyi(getResourcesDir() + "dragon.3d");
    mesh = std::make_shared<TriangleMesh>(tri_Object2World, plyi.nTriangles, plyi.vertexIndices, plyi.nVertices, plyi.vertexArray, nullptr, nullptr, nullptr, nullptr);
    tris.reserve(plyi.nTriangles);

    for (int i = 0; i < plyi.nTriangles; ++i)
        tris.push_back(std::make_shared<Triangle>(&tri_Object2World, &tri_World2Object, false, mesh, i));

    for (int i = 0; i < plyi.nTriangles; ++i)
        prims.push_back(std::make_shared<GeometricPrimitive>(tris[i], material, nullptr, MediumInterface()));
    plyi.Release();
}

void AddCornell(std::vector<std::shared_ptr<Primitive>> &prims,
                std::shared_ptr<Material> material1,
                std::shared_ptr<Material> material2,
                std::shared_ptr<Material> material3)
{
    //三角形个数
    const int nTrianglesWall = 2 * 5;
    int vertexIndicesWall[nTrianglesWall * 3];
    for (int i = 0; i < nTrianglesWall * 3; i++)
        vertexIndicesWall[i] = i;
    const int nVerticesWall = nTrianglesWall * 3;
    const float length_Wall = 5.0f;
    Point3f P_Wall[nVerticesWall] =
    {
        //底座
        Point3f(0.f,0.f,length_Wall),Point3f(length_Wall,0.f,length_Wall), Point3f(0.f,0.f,0.f),
        Point3f(length_Wall,0.f,length_Wall),Point3f(length_Wall,0.f,0.f),Point3f(0.f,0.f,0.f),
        //天花板
        Point3f(0.f,length_Wall,length_Wall),Point3f(0.f,length_Wall,0.f),Point3f(length_Wall,length_Wall,length_Wall),
        Point3f(length_Wall,length_Wall,length_Wall),Point3f(0.f,length_Wall,0.f),Point3f(length_Wall,length_Wall,0.f),
        //后墙
        Point3f(0.f,0.f,0.f),Point3f(length_Wall,0.f,0.f), Point3f(length_Wall,length_Wall,0.f),
        Point3f(0.f,0.f,0.f), Point3f(length_Wall,length_Wall,0.f),Point3f(0.f,length_Wall,0.f),
        //右墙
        Point3f(0.f,0.f,0.f),Point3f(0.f,length_Wall,length_Wall), Point3f(0.f,0.f,length_Wall),
        Point3f(0.f,0.f,0.f), Point3f(0.f,length_Wall,0.f),Point3f(0.f,length_Wall,length_Wall),
        //左墙
        Point3f(length_Wall,0.f,0.f),Point3f(length_Wall,length_Wall,length_Wall), Point3f(length_Wall,0.f,length_Wall),
        Point3f(length_Wall,0.f,0.f), Point3f(length_Wall,length_Wall,0.f),Point3f(length_Wall,length_Wall,length_Wall)
    };
    Transform tri_ConBox2World = Translate(Vector3f(-0.5*length_Wall,-0.5*length_Wall,-0.5*length_Wall));
    Transform tri_World2ConBox = Inverse(tri_ConBox2World);
    std::shared_ptr<TriangleMesh> meshConBox = std::make_shared<TriangleMesh>
        (tri_ConBox2World, nTrianglesWall, vertexIndicesWall, nVerticesWall, P_Wall, nullptr, nullptr, nullptr, nullptr);
    std::vector<std::shared_ptr<Shape>> trisConBox;
    for (int i = 0; i < nTrianglesWall; ++i)
        trisConBox.push_back(std::make_shared<Triangle>(&tri_ConBox2World, &tri_World2ConBox, false, meshConBox, i));

    //增加三角形到图元
    for (int i = 0; i < nTrianglesWall; ++i)
    {
        if (i == 6 || i == 7)
            prims.push_back(std::make_shared<GeometricPrimitive>(trisConBox[i], material1, nullptr, MediumInterface()));
        else if (i == 8 || i == 9)
            prims.push_back(std::make_shared<GeometricPrimitive>(trisConBox[i], material2, nullptr, MediumInterface()));
        else
            prims.push_back(std::make_shared<GeometricPrimitive>(trisConBox[i], material3, nullptr, MediumInterface()));
    }
}

void AddAreaLight(std::vector<std::shared_ptr<Primitive>> &prims, std::vector<std::shared_ptr<Light>>& lights,
                  std::shared_ptr<Material> material)
{
    int nTrianglesAreaLight = 2;
    int vertexIndicesAreaLight[6] = {0, 1, 2, 3, 4, 5};
    int nVerticesAreaLight = 6;
    const float yPos_AreaLight = 0.0;
    Point3f P_AreaLight[6] = { Point3f(-1.4,0.0,1.4), Point3f(-1.4,0.0,-1.4), Point3f(1.4,0.0,1.4),
        Point3f(1.4,0.0,1.4), Point3f(-1.4,0.0,-1.4), Point3f(1.4,0.0,-1.4)};
    Transform tri_Object2World_AreaLight = Translate(Vector3f(0.0f, 2.45f, 0.0f));
    Transform tri_World2Object_AreaLight = Inverse(tri_Object2World_AreaLight);

    std::shared_ptr<TriangleMesh> meshAreaLight = std::make_shared<TriangleMesh>
        (tri_Object2World_AreaLight, nTrianglesAreaLight, vertexIndicesAreaLight, nVerticesAreaLight, P_AreaLight, nullptr, nullptr, nullptr, nullptr);
    std::vector<std::shared_ptr<Shape>> trisAreaLight;
  
    for (int i = 0; i < nTrianglesAreaLight; ++i)
        trisAreaLight.push_back(std::make_shared<Triangle>(&tri_Object2World_AreaLight, &tri_World2Object_AreaLight, false, meshAreaLight, i));
    //
    for (int i = 0; i < nTrianglesAreaLight; ++i)
    {
        std::shared_ptr<AreaLight> area =
            std::make_shared<DiffuseAreaLight>(tri_Object2World_AreaLight, MediumInterface(), Spectrum(5.0f), 5, trisAreaLight[i], false);
        lights.push_back(area);
        prims.push_back(std::make_shared<GeometricPrimitive>(trisAreaLight[i], material, area, MediumInterface()));
    }
}

void AddSpotLight(std::vector<std::shared_ptr<Light>>& lights)
{
    Transform tri_Object2World_SpotLight = Translate(Vector3f(0.0f, 2.43f, 0.0f)) * Rotate(90, Vector3f(1.0f, 0.0f, 0.0f));
    std::shared_ptr<Light> spotLight = std::make_shared<SpotLight>(tri_Object2World_SpotLight, MediumInterface(), Spectrum(25.0f), 45, 30);
    lights.push_back(spotLight);
}

void AddDistLight(std::vector<std::shared_ptr<Light>>& lights)
{
    Transform tri_Object2World_DistantLight;
    std::shared_ptr<Light> distantLight = std::make_shared<DistantLight>(tri_Object2World_DistantLight, Spectrum(25.0f), Vector3f(0.0f, 0.0f, 1.0f));
    lights.push_back(distantLight);
}

void AddSkyLight(std::vector<std::shared_ptr<Light>>& lights)
{
    Transform SkyBoxToWorld;
    Point3f SkyBoxCenter(0.f, 0.f, 0.f);
    Float SkyBoxRadius = 10.0f;
    std::shared_ptr<Light> skyBoxLight = std::make_shared<SkyBoxLight>(SkyBoxToWorld, SkyBoxCenter, SkyBoxRadius, "1", 1);
    lights.push_back(skyBoxLight);
}

void AddInfLight(std::vector<std::shared_ptr<Light>>& lights)
{
    Transform InfinityLightToWorld = RotateX(20) * RotateY(-90) * RotateX(-90);
    Spectrum power(1.0f);
    std::shared_ptr<Light> infinityLight =
        std::make_shared<InfiniteAreaLight>(InfinityLightToWorld, power, 10, getResourcesDir() + "MonValley1000.hdr");
    lights.push_back(infinityLight);
}
