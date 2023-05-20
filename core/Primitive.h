#ifndef PBR_PRIMITIVE_INCLUDE_H
#define PBR_PRIMITIVE_INCLUDE_H

#include "Geometry.h"
#include "Material.h"


namespace pbr 
{

class Primitive 
{
  public:
    // Primitive Interface
    virtual ~Primitive();
    virtual Bounds3f WorldBound() const = 0;
    virtual bool Intersect(const Ray &r, SurfaceInteraction *) const = 0;
    virtual bool IntersectP(const Ray &r) const = 0;
    virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
};

class GeometricPrimitive : public Primitive 
{
public:
	// GeometricPrimitive Public Methods
	virtual Bounds3f WorldBound() const;
	virtual bool Intersect(const Ray &r, SurfaceInteraction *isect) const;
	virtual bool IntersectP(const Ray &r) const;
	GeometricPrimitive(const std::shared_ptr<Shape> &shape, const std::shared_ptr<Material> &material);
    virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const;
private:
	// GeometricPrimitive Private Data
	std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;
};


class Aggregate : public Primitive 
{
public:
	// Aggregate Public Methods
    virtual void ComputeScatteringFunctions(SurfaceInteraction *isect,
                                            MemoryArena &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const;

};


}

#endif
