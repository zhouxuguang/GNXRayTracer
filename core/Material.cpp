

// core/material.cpp*
#include "Material.h"
#include "Texture.h"
#include "Spectrum.h"
#include "Reflection.h"

namespace pbr
{

Material::~Material()
{
}

void Material::Bump(const std::shared_ptr<Texture<Float>> &d, SurfaceInteraction *si)
{
    // Compute offset positions and evaluate displacement texture
    SurfaceInteraction siEval = *si;

    // Shift _siEval_ _du_ in the $u$ direction
    Float du = .5f * (std::abs(si->dudx) + std::abs(si->dudy));
    // The most common reason for du to be zero is for ray that start from
    // light sources, where no differentials are available. In this case,
    // we try to choose a small enough du so that we still get a decently
    // accurate bump value.
    if (du == 0)
        du = .0005f;
    siEval.p = si->p + du * si->shading.dpdu;
    siEval.uv = si->uv + Vector2f(du, 0.f);
    siEval.n = Normalize((Normal3f)Cross(si->shading.dpdu, si->shading.dpdv) +
                         du * si->dndu);
    Float uDisplace = d->Evaluate(siEval);

    // Shift _siEval_ _dv_ in the $v$ direction
    Float dv = .5f * (std::abs(si->dvdx) + std::abs(si->dvdy));
    if (dv == 0)
        dv = .0005f;
    siEval.p = si->p + dv * si->shading.dpdv;
    siEval.uv = si->uv + Vector2f(0.f, dv);
    siEval.n = Normalize((Normal3f)Cross(si->shading.dpdu, si->shading.dpdv) +
                         dv * si->dndv);
    Float vDisplace = d->Evaluate(siEval);
    Float displace = d->Evaluate(*si);

    // Compute bump-mapped differential geometry
    Vector3f dpdu = si->shading.dpdu +
                    (uDisplace - displace) / du * Vector3f(si->shading.n) +
                    displace * Vector3f(si->shading.dndu);
    Vector3f dpdv = si->shading.dpdv +
                    (vDisplace - displace) / dv * Vector3f(si->shading.n) +
                    displace * Vector3f(si->shading.dndv);
    si->SetShadingGeometry(dpdu, dpdv, si->shading.dndu, si->shading.dndv, false);
}

void Material::NormalMap(const std::shared_ptr<Texture<Spectrum>> &normalMap,
                         SurfaceInteraction *si, Float bumpScale)
{
    // 1) 取切空间法线
    Float rgb[3];
    normalMap->Evaluate(*si).ToRGB(rgb);
    Vector3f nT(2.f * rgb[0] - 1.f, 2.f * rgb[1] - 1.f, 2.f * rgb[2] - 1.f);

    // <BumpScale>： mix((0,0,1), n, bumpScale)。
    nT = Vector3f(0, 0, 1) * (1.0 - bumpScale) + nT * bumpScale;
    
    if (nT.LengthSquared() < 1e-8f)
        return;
    nT = Normalize(nT);
    
    // 2) 用几何法线与几何切线构造 TBN
    Vector3f ns = Normalize(Vector3f(si->n));
    Vector3f bs = Normalize(Cross(ns, si->dpdu));
    Vector3f ts = Normalize(Cross(bs, ns));
    bs = Normalize(bs);
    
    // 3) 切空间 → 世界空间
    Vector3f n = Normalize(Vector3f(ts.x * nT.x + bs.x * nT.y + ns.x * nT.z,
                          ts.y * nT.x + bs.y * nT.y + ns.y * nT.z,
                          ts.z * nT.x + bs.z * nT.y + ns.z * nT.z));

    if (n.LengthSquared() == 0.f)
    {
        return;
    }
    
    // 4) 只更新着色法线
    Vector3f newNs = Faceforward(Normalize(n), si->n);
    si->shading.n = Normal3f(newNs);
}

} // namespace pbr
