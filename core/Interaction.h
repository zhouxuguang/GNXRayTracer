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
	void ComputeScatteringFunctions();
	const Shape *shape = nullptr;
	const Primitive *primitive = nullptr;
    
    std::shared_ptr<BSDF> bsdf = nullptr;
    Point2f uv;
    Vector3f dpdu, dpdv;
    Normal3f dndu, dndv;
    struct
    {
        Normal3f n;
        Vector3f dpdu, dpdv;
        Normal3f dndu, dndv;
    } shading;
};

}


#endif // PBR_INTERACTION_CINDSFMG_H


