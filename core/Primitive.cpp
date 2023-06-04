#include "Primitive.h"
#include "Shape.h"
#include "Interaction.h"


namespace pbr 
{
static long long primitiveMemory = 0;

Primitive::~Primitive() {}

// GeometricPrimitive Method Definitions
GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape, const std::shared_ptr<Material> &material,
                                       const std::shared_ptr<AreaLight> &areaLight) :
    shape(shape), material(material), areaLight(areaLight)
{
    primitiveMemory += sizeof(*this);
}

Bounds3f GeometricPrimitive::WorldBound() const { return shape->WorldBound(); }

bool GeometricPrimitive::IntersectP(const Ray &r) const
{
    return shape->IntersectP(r);
}

bool GeometricPrimitive::Intersect(const Ray &r, SurfaceInteraction *isect) const
{
    float tHit;
    if (!shape->Intersect(r, &tHit, isect)) return false;
    r.tMax = tHit;
    isect->primitive = this;
    
    CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
    assert(Dot(isect->n, isect->shading.n) >= 0.0);
    
    // Initialize _SurfaceInteraction::mediumInterface_ after _Shape_
    // intersection
    return true;
}

const AreaLight *GeometricPrimitive::GetAreaLight() const
{
    return areaLight.get();
}

const Material *GeometricPrimitive::GetMaterial() const
{
    return material.get();
}

void GeometricPrimitive::ComputeScatteringFunctions(
                                                    SurfaceInteraction *isect,
                                                    MemoryArena &arena,
                                                    TransportMode mode,
                                                    bool allowMultipleLobes) const
{
    //ProfilePhase p(Prof::ComputeScatteringFuncs);
    if (material)
        material->ComputeScatteringFunctions(isect, arena, mode,
                                             allowMultipleLobes);
    CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
    assert(Dot(isect->n, isect->shading.n) >= 0.0);
}

const AreaLight *Aggregate::GetAreaLight() const
{
    return nullptr;
}

const Material *Aggregate::GetMaterial() const
{
//    LOG(FATAL) <<
//        "Aggregate::GetMaterial() method"
//        "called; should have gone to GeometricPrimitive";
    return nullptr;
}

void Aggregate::ComputeScatteringFunctions(SurfaceInteraction *isect,
                                           MemoryArena &arena,
                                           TransportMode mode,
                                           bool allowMultipleLobes) const
{
    //Aggregate类无需实现具体的计算散射的函数，因为只是个加速结构
//    LOG(FATAL) <<
//        "Aggregate::ComputeScatteringFunctions() method"
//        "called; should have gone to GeometricPrimitive";
}

}
