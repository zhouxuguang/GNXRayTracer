

// lights/diffuse.cpp*
#include "lights/DiffuseAreaLight.h"
#include "core/Sampling.h"
#include "shape/Triangle.h"

namespace pbr
{

DiffuseAreaLight::DiffuseAreaLight(const Transform &LightToWorld, const MediumInterface &mediumInterface,
                                   const Spectrum &Lemit, int nSamples,
                                   const std::shared_ptr<Shape> &shape)
    : AreaLight(LightToWorld, mediumInterface, nSamples),
      mLemit(Lemit),
      mShape(shape),
      mArea(shape->Area())
{
}

pbr::Spectrum DiffuseAreaLight::L(const Interaction& intr, const Vector3f& w) const
{
	bool dotNW = Dot(intr.n, w);
	return (dotNW > 0) ? mLemit : Spectrum(0.f);
}

Spectrum DiffuseAreaLight::Power() const
{
    return mLemit * mArea * Pi;
}

Spectrum DiffuseAreaLight::Sample_Li(const Interaction &ref, const Point2f &u,
                                     Vector3f *wi, Float *pdf,
                                     VisibilityTester *vis) const
{
    Interaction pShape = mShape->Sample(ref, u, pdf);
    pShape.mediumInterface = mediumInterface;
    if (*pdf == 0 || (pShape.p - ref.p).LengthSquared() == 0) //如果pdf为0或者相交点和光源距离非常近
    {
        *pdf = 0;
        return 0.f;
    }
    *wi = Normalize(pShape.p - ref.p);    //从相交点指向光源
    *vis = VisibilityTester(ref, pShape); //可见性测试对象
    return L(pShape, -*wi);               //计算光源发射的光亮度
}

Float DiffuseAreaLight::Pdf_Li(const Interaction &ref, const Vector3f &wi) const
{
    return mShape->Pdf(ref, wi);
}

Spectrum DiffuseAreaLight::Sample_Le(const Point2f &u1, const Point2f &u2,
                                     Float time, Ray *ray, Normal3f *nLight,
                                     Float *pdfPos, Float *pdfDir) const
{
    // Sample a point on the area light's _Shape_, _pShape_
    Interaction pShape = mShape->Sample(u1, pdfPos);
    pShape.mediumInterface = mediumInterface;
    *nLight = pShape.n;

    // Sample a cosine-weighted outgoing direction _w_ for area light
    Vector3f w = CosineSampleHemisphere(u2);
    *pdfDir = CosineHemispherePdf(w.z);

    Vector3f v1, v2, n(pShape.n);
    CoordinateSystem(n, &v1, &v2);
    w = w.x * v1 + w.y * v2 + w.z * n;
    *ray = pShape.SpawnRay(w);
    return L(pShape, w);
}

void DiffuseAreaLight::Pdf_Le(const Ray &ray, const Normal3f &n, Float *pdfPos, Float *pdfDir) const
{
    Interaction it(ray.o, n, Vector3f(), Vector3f(n), ray.time, mediumInterface);
    *pdfPos = mShape->Pdf(it);
    *pdfDir = CosineHemispherePdf(Dot(n, ray.d));
}

} // namespace pbr
