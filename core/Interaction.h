#ifndef PBR_INTERACTION_CINDSFMG_H
#define PBR_INTERACTION_CINDSFMG_H

#include "Shape.h"
#include "Primitive.h"
#include "Spectrum.h"

namespace pbr 
{

struct Interaction
{
	// Interaction Public Methods
	Interaction() : time(0) {}
    
    Interaction(const Point3f &p, const Vector3f &wo, Float time)
        : p(p), time(time), wo(wo) {}
    
    Interaction(const Point3f &p, Float time)
        : p(p), time(time) {}
    
	Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
		const Vector3f &wo, float time)
		: p(p),
		time(time),
        pError(pError),
		wo(Normalize(wo)),
		n(n) {}
    
    Ray SpawnRay(const Vector3f &d) const
    {
        Point3f o = OffsetRayOrigin(p, pError, n, d);
        return Ray(o, d, Infinity, time, GetMedium(d));
    }
    
    Ray SpawnRayTo(const Point3f &p2) const
    {
        Point3f origin = OffsetRayOrigin(p, pError, n, p2 - p);
        Vector3f d = p2 - p;
        return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
    }
    
    Ray SpawnRayTo(const Interaction &it) const
    {
        // 计算交点和光源点之间的射线，用于阴影检测
        Point3f origin = OffsetRayOrigin(p, pError, n, it.p - p);
        Point3f target = OffsetRayOrigin(it.p, it.pError, it.n, origin - it.p);
        Vector3f d = target - origin;
        return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
    }
    
    bool IsMediumInteraction() const
    {
        return false;
        //return !IsSurfaceInteraction();
    }
    
    const Medium *GetMedium(const Vector3f &w) const
    {
        return nullptr;
        //return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
    }
    
    const Medium *GetMedium() const
    {
        CHECK_EQ(mediumInterface.inside, mediumInterface.outside);
        return nullptr;
        //return mediumInterface.inside;
    }
    
	// Interaction Public Data
	Point3f p;
	float time;
	Vector3f wo;
	Normal3f n;
    Vector3f pError;
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
    
    Spectrum Le(const Vector3f &w) const;
    
    
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
