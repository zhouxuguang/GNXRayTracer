#ifndef PBR_PRIMITIVE_INCLUDE_H
#define PBR_PRIMITIVE_INCLUDE_H

#include "Geometry.h"
#include "Material.h"
#include "Medium.h"
#include "Transform.h"


namespace pbr
{

// 图元基类
class Primitive
{
public:
    virtual ~Primitive();
    virtual Bounds3f WorldBound() const = 0;
    virtual bool Intersect(const Ray &r, SurfaceInteraction *) const = 0;
    virtual bool IntersectP(const Ray &r) const = 0;
    virtual const AreaLight *GetAreaLight() const = 0;
    virtual const Material *GetMaterial() const = 0;
    virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
};

// 几何图元
class GeometricPrimitive : public Primitive
{
public:
    virtual Bounds3f WorldBound() const;
    virtual bool Intersect(const Ray &r, SurfaceInteraction *isect) const;
    virtual bool IntersectP(const Ray &r) const;
    GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                       const std::shared_ptr<Material> &material,
                       const std::shared_ptr<AreaLight> &areaLight,
                       const MediumInterface &mediumInterface);
    const AreaLight *GetAreaLight() const;
    const Material *GetMaterial() const;
    void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;
    std::shared_ptr<AreaLight> areaLight;
    MediumInterface mediumInterface;
};

// 变换图元
class TransformedPrimitive : public Primitive
{
public:
    TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                         const AnimatedTransform &PrimitiveToWorld);
    bool Intersect(const Ray &r, SurfaceInteraction *in) const;
    bool IntersectP(const Ray &r) const;
    const AreaLight *GetAreaLight() const
    {
        return nullptr;
    }
    const Material *GetMaterial() const
    {
        return nullptr;
    }
    void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                    MemoryArena &arena, TransportMode mode,
                                    bool allowMultipleLobes) const
    {
    }
    Bounds3f WorldBound() const
    {
        return PrimitiveToWorld.MotionBounds(primitive->WorldBound());
    }

private:
    std::shared_ptr<Primitive> primitive;
    const AnimatedTransform PrimitiveToWorld;
};

// 加速结构图元
class Aggregate : public Primitive
{
public:
    const AreaLight *GetAreaLight() const;
    const Material *GetMaterial() const;
    virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const;
};


}

#endif
