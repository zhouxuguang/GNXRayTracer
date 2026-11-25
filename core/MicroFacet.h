
#ifndef PBR_CORE_MICROFACET_H
#define PBR_CORE_MICROFACET_H

// core/microfacet.h*
#include "Geometry.h"

namespace pbr
{

// 微表面分布基类
class MicrofacetDistribution
{
public:
    virtual ~MicrofacetDistribution();
    virtual Float D(const Vector3f &wh) const = 0;
    virtual Float Lambda(const Vector3f &w) const = 0;
    Float G1(const Vector3f &w) const
    {
        return 1 / (1 + Lambda(w));
    }
    virtual Float G(const Vector3f &wo, const Vector3f &wi) const
    {
        return 1 / (1 + Lambda(wo) + Lambda(wi));
    }
    virtual Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const = 0;
    Float Pdf(const Vector3f &wo, const Vector3f &wh) const;
    virtual std::string ToString() const = 0;

protected:
    MicrofacetDistribution(bool sampleVisibleArea)
        : sampleVisibleArea(sampleVisibleArea)
    {
    }

    const bool sampleVisibleArea;
};

inline std::ostream &operator<<(std::ostream &os, const MicrofacetDistribution &md)
{
    os << md.ToString();
    return os;
}

class BeckmannDistribution : public MicrofacetDistribution
{
public:
    static Float RoughnessToAlpha(Float roughness)
    {
        roughness = std::max(roughness, (Float)1e-3);
        Float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    }
    BeckmannDistribution(Float alphax, Float alphay, bool samplevis = true)
        : MicrofacetDistribution(samplevis),
          alphax(std::max(Float(0.001), alphax)),
          alphay(std::max(Float(0.001), alphay))
    {
    }
    Float D(const Vector3f &wh) const;
    Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const;
    std::string ToString() const;

private:
    Float Lambda(const Vector3f &w) const;

    const Float alphax, alphay;
};

class TrowbridgeReitzDistribution : public MicrofacetDistribution
{
public:
    static inline Float RoughnessToAlpha(Float roughness);
    TrowbridgeReitzDistribution(Float alphax, Float alphay,
                                bool samplevis = true)
        : MicrofacetDistribution(samplevis),
          alphax(std::max(Float(0.001), alphax)),
          alphay(std::max(Float(0.001), alphay))
    {
    }
    Float D(const Vector3f &wh) const;
    Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const;
    std::string ToString() const;

private:
    Float Lambda(const Vector3f &w) const;

    const Float alphax, alphay;
};

inline Float TrowbridgeReitzDistribution::RoughnessToAlpha(Float roughness)
{
    roughness = std::max(roughness, (Float)1e-3);
    Float x = std::log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
           0.000640711f * x * x * x * x;
}

} // namespace pbr

#endif // PBR_CORE_MICROFACET_H
