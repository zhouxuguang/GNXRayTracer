#include "SkyBoxLight.h"
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "3rd/stb_image.h"

namespace pbr
{

static void get_sphere_uv(const Vector3f &p, float &u, float &v)
{
	float phi = atan2(p.z, p.x);
	float theta = asin(p.y);
	u = 1 - (phi + Pi) * Inv2Pi;
	v = (theta + PiOver2) * InvPi;
}

bool SkyBoxLight::loadImage(const char* imageFile)
{
	stbi_set_flip_vertically_on_load(true);
	data = stbi_loadf(imageFile, &imageWidth, &imageHeight, &nrComponents, 0);
	if (data)return true;
	else return false;
}

Spectrum SkyBoxLight::getLightValue(float u, float v) const
{
	int w = u * imageWidth, h = v * imageHeight;
	int offset = (w + h * imageWidth) * nrComponents;
	Spectrum Lv(0.0);
    
    if (data)
    {
        const float scale = 1.0f / 10.0f;
        Lv[0] = data[offset + 0] * scale;
        Lv[1] = data[offset + 1] * scale;
        Lv[2] = data[offset + 2] * scale;
    }
	
	return Lv;
}

Spectrum SkyBoxLight::Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
	float *pdf, VisibilityTester *vis) const
{
	float theta = u.y * Pi, phi = u.x * 2 * Pi;
	float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
	float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
	*wi = LightToWorld(Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
	*pdf = 1.f / (4 * Pi);
	*vis = VisibilityTester(ref, Interaction(ref.p + *wi * (2 * worldRadius), ref.time));
	return 16 * getLightValue(u.x, u.y);
}

Spectrum SkyBoxLight::Le(const RayDifferential &ray) const
{
	Vector3f oc = ray.o - worldCenter;
	float a = Dot(ray.d, ray.d);
	float b = 2.0 * Dot(oc, ray.d);
	float c = Dot(oc, oc) - worldRadius * worldRadius;
	float discriminant = b*b - 4 * a*c;
	float t;
	if (discriminant < 0) 
		return 0.f;
	
	t = (-b + sqrt(discriminant)) / (2.0 * a);

	Point3f hitPos = ray.o + t * ray.d;
	//偏移到使球心到坐标原点
	Vector3f hitPos_temp = hitPos - worldCenter;

	float u, v;
	get_sphere_uv(hitPos_temp / worldRadius, u, v);

	Spectrum Col;
	if (data) {
		Col = getLightValue(u, v);
	}
	else {
		Col[0] = (hitPos_temp.x + worldRadius) / (2.f * worldRadius);
		Col[1] = (hitPos_temp.y + worldRadius) / (2.f * worldRadius);
		Col[2] = (hitPos_temp.z + worldRadius) / (2.f * worldRadius);
	}
	return Col;
}

}
