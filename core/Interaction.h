#ifndef PBR_INTERACTION_CINDSFMG_H
#define PBR_INTERACTION_CINDSFMG_H

#include "Shape.h"
#include "Primitive.h"

namespace pbr 
{

struct Interaction
{
	// Interaction Public Methods
	Interaction() : time(0) {}
	Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
		const Vector3f &wo, float time)
		: p(p),
		time(time),
		wo(Normalize(wo)),
		n(n) {}
    
	// Interaction Public Data
	Point3f p;
	float time;
	Vector3f wo;
	Normal3f n;
};

class SurfaceInteraction : public Interaction
{
public:
	// SurfaceInteraction Public Methods
	SurfaceInteraction() {}
    
    SurfaceInteraction(const Point3f &p, const Vector3f &pError,
                       const Point2f &uv, const Vector3f &wo,
                       const Vector3f &dpdu, const Vector3f &dpdv,
                       const Normal3f &dndu, const Normal3f &dndv, Float time,
                       const Shape *sh,
                       int faceIndex = 0);
    
    void SetShadingGeometry(const Vector3f &dpdu, const Vector3f &dpdv,
                            const Normal3f &dndu, const Normal3f &dndv,
                            bool orientationIsAuthoritative);
    
    void ComputeScatteringFunctions(
        const Ray &ray, MemoryArena &arena,
        bool allowMultipleLobes = false,
        TransportMode mode = TransportMode::Radiance);
    
    
	const Shape *shape = nullptr;
    
    Point2f uv;
    Vector3f dpdu, dpdv;
    Normal3f dndu, dndv;
    struct {
        Normal3f n;
        Vector3f dpdu, dpdv;
        Normal3f dndu, dndv;
    } shading;
    const Primitive *primitive = nullptr;
    BSDF *bsdf = nullptr;
    BSSRDF *bssrdf = nullptr;
    mutable Vector3f dpdx, dpdy;
    mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
};

}


#endif // PBR_INTERACTION_CINDSFMG_H
