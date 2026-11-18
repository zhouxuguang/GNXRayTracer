#ifndef PBR_SHAPES_TRIANGLE_H
#define PBR_SHAPES_TRIANGLE_H

// shapes/triangle.h*
#include "core/Shape.h"
#include <map>

namespace pbr
{

// 三角网格定义
struct TriangleMesh
{
    TriangleMesh(const Transform &ObjectToWorld, int nTriangles,
                 const int *vertexIndices, int nVertices, const Point3f *P,
                 const Vector3f *S, const Normal3f *N, const Point2f *uv,
                 const int *faceIndices);

    const int nTriangles, nVertices;   //三角形数量和顶点数量
    std::vector<int> vertexIndices;    //顶点索引数组
    std::unique_ptr<Point3f[]> p;      //顶点位置数组
    std::unique_ptr<Normal3f[]> n;     //顶点法线数组
    std::unique_ptr<Vector3f[]> s;     //顶点切线数组
    std::unique_ptr<Point2f[]> uv;     //顶点纹理坐标数组
    std::shared_ptr<Texture<Float>> alphaMask, shadowAlphaMask;  //遮罩纹理
    std::vector<int> faceIndices;      //三角形索引数组
};

//三角形定义
class Triangle : public Shape
{
public:
    Triangle(const Transform *ObjectToWorld, const Transform *WorldToObject,
             bool reverseOrientation, const std::shared_ptr<TriangleMesh> &mesh,
             int triNumber)
        : Shape(ObjectToWorld, WorldToObject, reverseOrientation), mMesh(mesh)
    {
        mV = &mesh->vertexIndices[3 * triNumber];
        mFaceIndex = mesh->faceIndices.size() ? mesh->faceIndices[triNumber] : 0;
    }
    Bounds3f ObjectBound() const;
    Bounds3f WorldBound() const;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture = true) const;
    bool IntersectP(const Ray &ray, bool testAlphaTexture = true) const;
    Float Area() const;

    //using Shape::Sample;  // Bring in the other Sample() overload.
    Interaction Sample(const Point2f &u, Float *pdf) const;

    // Returns the solid angle subtended by the triangle w.r.t. the given
    // reference point p.
    Float SolidAngle(const Point3f &p, int nSamples = 0) const;

private:
    // Triangle Private Methods
    void GetUVs(Point2f uv[3]) const
    {
        if (mMesh->uv)
        {
            uv[0] = mMesh->uv[mV[0]];
            uv[1] = mMesh->uv[mV[1]];
            uv[2] = mMesh->uv[mV[2]];
        }
        else
        {
            uv[0] = Point2f(0, 0);
            uv[1] = Point2f(1, 0);
            uv[2] = Point2f(1, 1);
        }
    }

    std::shared_ptr<TriangleMesh> mMesh;  //所在的mesh
    const int *mV;
    int mFaceIndex;
};

}

#endif
