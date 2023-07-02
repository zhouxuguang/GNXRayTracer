

#ifndef PBR_CORE_SUBREFLECTION_H
#define PBR_CORE_SUBREFLECTION_H

// core/bssrdf.h*
#include "Interaction.h"
#include "Reflection.h"

namespace pbr
{

// BSSRDF Utility Declarations
Float FresnelMoment1(Float invEta);
Float FresnelMoment2(Float invEta);

// BSSRDF Declarations
class BSSRDF
{
public:
    // BSSRDF Public Methods
    BSSRDF(const SurfaceInteraction &po, Float eta) : po(po), eta(eta) {}
    virtual ~BSSRDF() {}

    // BSSRDF Interface
    virtual Spectrum S(const SurfaceInteraction &pi, const Vector3f &wi) = 0;
    virtual Spectrum Sample_S(const Scene &scene, Float u1, const Point2f &u2,
                              MemoryArena &arena, SurfaceInteraction *si,
                              Float *pdf) const = 0;

protected:
    // BSSRDF Protected Data
    const SurfaceInteraction &po;
    Float eta;
};

class SeparableBSSRDF : public BSSRDF
{
    friend class SeparableBSSRDFAdapter;

public:
    // SeparableBSSRDF Public Methods
    SeparableBSSRDF(const SurfaceInteraction &po, Float eta,
                    const Material *material, TransportMode mode)
        : BSSRDF(po, eta),
          ns(po.shading.n),
          ss(Normalize(po.shading.dpdu)),
          ts(Cross(ns, ss)),
          material(material),
          mode(mode) {}
    Spectrum S(const SurfaceInteraction &pi, const Vector3f &wi) {
        //ProfilePhase pp(Prof::BSSRDFEvaluation);
        Float Ft = FrDielectric(CosTheta(po.wo), 1, eta);
        return (1 - Ft) * Sp(pi) * Sw(wi);
    }
    Spectrum Sw(const Vector3f &w) const {
        Float c = 1 - 2 * FresnelMoment1(1 / eta);
        return (1 - FrDielectric(CosTheta(w), 1, eta)) / (c * Pi);
    }
    Spectrum Sp(const SurfaceInteraction &pi) const {
        return Sr(Distance(po.p, pi.p));
    }
    Spectrum Sample_S(const Scene &scene, Float u1, const Point2f &u2,
                      MemoryArena &arena, SurfaceInteraction *si,
                      Float *pdf) const;
    Spectrum Sample_Sp(const Scene &scene, Float u1, const Point2f &u2,
                       MemoryArena &arena, SurfaceInteraction *si,
                       Float *pdf) const;
    Float Pdf_Sp(const SurfaceInteraction &si) const;

    // SeparableBSSRDF Interface
    virtual Spectrum Sr(Float d) const = 0;
    virtual Float Sample_Sr(int ch, Float u) const = 0;
    virtual Float Pdf_Sr(int ch, Float r) const = 0;

private:
    // SeparableBSSRDF Private Data
    const Normal3f ns;
    const Vector3f ss, ts;
    const Material *material;
    const TransportMode mode;
};

class TabulatedBSSRDF : public SeparableBSSRDF
{
public:
    // TabulatedBSSRDF Public Methods
    TabulatedBSSRDF(const SurfaceInteraction &po, const Material *material,
                    TransportMode mode, Float eta, const Spectrum &sigma_a,
                    const Spectrum &sigma_s, const BSSRDFTable &table)
        : SeparableBSSRDF(po, eta, material, mode), table(table) {
        sigma_t = sigma_a + sigma_s;
        for (int c = 0; c < Spectrum::nSamples; ++c)
            rho[c] = sigma_t[c] != 0 ? (sigma_s[c] / sigma_t[c]) : 0;
    }
    Spectrum Sr(Float distance) const;
    Float Pdf_Sr(int ch, Float distance) const;
    Float Sample_Sr(int ch, Float sample) const;

private:
    // TabulatedBSSRDF Private Data
    const BSSRDFTable &table;
    Spectrum sigma_t, rho;
};

struct BSSRDFTable {
    // BSSRDFTable Public Data
    const int nRhoSamples, nRadiusSamples;
    std::unique_ptr<Float[]> rhoSamples, radiusSamples;
    std::unique_ptr<Float[]> profile;
    std::unique_ptr<Float[]> rhoEff;
    std::unique_ptr<Float[]> profileCDF;

    // BSSRDFTable Public Methods
    BSSRDFTable(int nRhoSamples, int nRadiusSamples);
    inline Float EvalProfile(int rhoIndex, int radiusIndex) const {
        return profile[rhoIndex * nRadiusSamples + radiusIndex];
    }
};

class SeparableBSSRDFAdapter : public BxDF
{
public:
    // SeparableBSSRDFAdapter Public Methods
    SeparableBSSRDFAdapter(const SeparableBSSRDF *bssrdf)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), bssrdf(bssrdf) {}
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        Spectrum f = bssrdf->Sw(wi);
        // Update BSSRDF transmission term to account for adjoint light
        // transport
        if (bssrdf->mode == TransportMode::Radiance)
            f *= bssrdf->eta * bssrdf->eta;
        return f;
    }
    std::string ToString() const { return "[ SeparableBSSRDFAdapter ]"; }

private:
    const SeparableBSSRDF *bssrdf;
};

Float BeamDiffusionSS(Float sigma_s, Float sigma_a, Float g, Float eta,
                      Float r);
Float BeamDiffusionMS(Float sigma_s, Float sigma_a, Float g, Float eta,
                      Float r);
void ComputeBeamDiffusionBSSRDF(Float g, Float eta, BSSRDFTable *t);
void SubsurfaceFromDiffuse(const BSSRDFTable &table, const Spectrum &rhoEff,
                           const Spectrum &mfp, Spectrum *sigma_a,
                           Spectrum *sigma_s);

}  // namespace pbr

#endif  // PBR_CORE_SUBREFLECTION_H
