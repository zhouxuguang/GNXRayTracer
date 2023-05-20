#include "Primitive.h"
#include "Shape.h"
#include "Interaction.h"


namespace pbr 
{
static long long primitiveMemory = 0;

Primitive::~Primitive() {}

// GeometricPrimitive Method Definitions
GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape, const std::shared_ptr<Material> &material) :
    shape(shape), material(material)
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
