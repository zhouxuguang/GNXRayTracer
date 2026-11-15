#include "Sampling.h"


namespace pbr
{

// Sampling Function Definitions
void StratifiedSample1D(Float *samp, int nSamples, RNG &rng, bool jitter)
{
    Float invNSamples = (Float)1 / nSamples;
    for (int i = 0; i < nSamples; ++i)
    {
        Float delta = jitter ? rng.UniformFloat() : 0.5f;
        samp[i] = std::min((i + delta) * invNSamples, OneMinusEpsilon);
    }
}

void StratifiedSample2D(Point2f *samp, int nx, int ny, RNG &rng, bool jitter)
{
    Float dx = (Float)1 / nx, dy = (Float)1 / ny;
    for (int y = 0; y < ny; ++y)
    {
        for (int x = 0; x < nx; ++x)
        {
            Float jx = jitter ? rng.UniformFloat() : 0.5f;
            Float jy = jitter ? rng.UniformFloat() : 0.5f;
            samp->x = std::min((x + jx) * dx, OneMinusEpsilon);
            samp->y = std::min((y + jy) * dy, OneMinusEpsilon);
            ++samp;
        }
    }
}

void LatinHypercube(Float *samples, int nSamples, int nDim, RNG &rng)
{
    // Generate LHS samples along diagonal
    Float invNSamples = (Float)1 / nSamples;
    for (int i = 0; i < nSamples; ++i)
        for (int j = 0; j < nDim; ++j)
        {
            Float sj = (i + (rng.UniformFloat())) * invNSamples;
            samples[nDim * i + j] = std::min(sj, OneMinusEpsilon);
        }

    // Permute LHS samples in each dimension
    for (int i = 0; i < nDim; ++i)
    {
        for (int j = 0; j < nSamples; ++j)
        {
            int other = j + rng.UniformUInt32(nSamples - j);
            std::swap(samples[nDim * j + i], samples[nDim * other + i]);
        }
    }
}

Distribution1D::Distribution1D(const Float *f, int n)
    : func(f, f + n), cdf(n + 1)
{
    // Compute integral of step function at $x_i$
    cdf[0] = 0;
    for (int i = 1; i < n + 1; ++i)
        cdf[i] = cdf[i - 1] + func[i - 1] / n;

    // Transform step function integral into CDF
    funcInt = cdf[n];
    if (funcInt == 0)
    {
        for (int i = 1; i < n + 1; ++i)
            cdf[i] = Float(i) / Float(n);
    }
    else
    {
        for (int i = 1; i < n + 1; ++i)
            cdf[i] /= funcInt;
    }
}

int Distribution1D::Count() const
{
    return (int)func.size();
}

Float Distribution1D::SampleContinuous(Float u, Float *pdf, int *off) const
{
    // Find surrounding CDF segments and _offset_
    int offset = FindInterval((int)cdf.size(),
                              [&](int index)
                              {
                                  return cdf[index] <= u;
                              });
    if (off)
        *off = offset;
    // Compute offset along CDF segment
    Float du = u - cdf[offset];
    if ((cdf[offset + 1] - cdf[offset]) > 0)
    {
        CHECK_GT(cdf[offset + 1], cdf[offset]);
        du /= (cdf[offset + 1] - cdf[offset]);
    }
    DCHECK(!std::isnan(du));

    // Compute PDF for sampled offset
    if (pdf)
        *pdf = (funcInt > 0) ? func[offset] / funcInt : 0;

    // Return $x\in{}[0,1)$ corresponding to sample
    return (offset + du) / Count();
}

int Distribution1D::SampleDiscrete(Float u, Float *pdf,
                                   Float *uRemapped) const
{
    // Find surrounding CDF segments and _offset_
    int offset = FindInterval((int)cdf.size(),
                              [&](int index)
                              {
                                  return cdf[index] <= u;
                              });
    if (pdf)
        *pdf = (funcInt > 0) ? func[offset] / (funcInt * Count()) : 0;
    if (uRemapped)
        *uRemapped = (u - cdf[offset]) / (cdf[offset + 1] - cdf[offset]);
    if (uRemapped)
        CHECK(*uRemapped >= 0.f && *uRemapped <= 1.f);
    return offset;
}

Float Distribution1D::DiscretePDF(int index) const
{
    CHECK(index >= 0 && index < Count());
    return func[index] / (funcInt * Count());
}

//二维圆盘上的拒绝采样
Point2f RejectionSampleDisk(RNG &rng)
{
    Point2f p;
    do
    {
        p.x = 1 - 2 * rng.UniformFloat();
        p.y = 1 - 2 * rng.UniformFloat();
    } while (p.x * p.x + p.y * p.y > 1);
    return p;
}

Vector3f UniformSampleHemisphere(const Point2f &u)
{
    Float z = u[0];
    Float r = std::sqrt(std::max((Float)0, (Float)1. - z * z));
    Float phi = 2 * Pi * u[1];
    return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
}

Float UniformHemispherePdf()
{
    return Inv2Pi;
}

Vector3f UniformSampleSphere(const Point2f &u)
{
    Float z = 1 - 2 * u[0];
    Float r = std::sqrt(std::max((Float)0, (Float)1 - z * z));
    Float phi = 2 * Pi * u[1];
    return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
}

Float UniformSpherePdf()
{
    return Inv4Pi;
}

Point2f UniformSampleDisk(const Point2f &u)
{
    Float r = std::sqrt(u[0]);
    Float theta = 2 * Pi * u[1];
    return Point2f(r * std::cos(theta), r * std::sin(theta));
}

Point2f ConcentricSampleDisk(const Point2f &u)
{
    // Map uniform random numbers to $[-1,1]^2$
    Point2f uOffset = 2.f * u - Vector2f(1, 1);

    // Handle degeneracy at the origin
    if (uOffset.x == 0 && uOffset.y == 0)
        return Point2f(0, 0);

    // Apply concentric mapping to point
    Float theta, r;
    if (std::abs(uOffset.x) > std::abs(uOffset.y))
    {
        r = uOffset.x;
        theta = PiOver4 * (uOffset.y / uOffset.x);
    }
    else
    {
        r = uOffset.y;
        theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
    }
    return r * Point2f(std::cos(theta), std::sin(theta));
}

Float UniformConePdf(Float cosThetaMax)
{
    return 1 / (2 * Pi * (1 - cosThetaMax));
}

Vector3f UniformSampleCone(const Point2f &u, Float cosThetaMax)
{
    Float cosTheta = ((Float)1 - u[0]) + u[0] * cosThetaMax;
    Float sinTheta = std::sqrt((Float)1 - cosTheta * cosTheta);
    Float phi = u[1] * 2 * Pi;
    return Vector3f(std::cos(phi) * sinTheta, std::sin(phi) * sinTheta,
                    cosTheta);
}

Vector3f UniformSampleCone(const Point2f &u, Float cosThetaMax,
                           const Vector3f &x, const Vector3f &y,
                           const Vector3f &z)
{
    Float cosTheta = Lerp(u[0], cosThetaMax, 1.f);
    Float sinTheta = std::sqrt((Float)1. - cosTheta * cosTheta);
    Float phi = u[1] * 2 * Pi;
    return std::cos(phi) * sinTheta * x + std::sin(phi) * sinTheta * y +
           cosTheta * z;
}

Point2f UniformSampleTriangle(const Point2f &u)
{
    Float su0 = std::sqrt(u[0]);
    return Point2f(1 - su0, u[1] * su0);
}

Distribution2D::Distribution2D(const Float *func, int nu, int nv)
{
    pConditionalV.reserve(nv);
    for (int v = 0; v < nv; ++v)
    {
        // Compute conditional sampling distribution for $\tilde{v}$
        pConditionalV.emplace_back(new Distribution1D(&func[v * nu], nu));
    }
    // Compute marginal sampling distribution $p[\tilde{v}]$
    std::vector<Float> marginalFunc;
    marginalFunc.reserve(nv);
    for (int v = 0; v < nv; ++v)
    {
        marginalFunc.push_back(pConditionalV[v]->funcInt);
    }
    pMarginal.reset(new Distribution1D(&marginalFunc[0], nv));
}

Point2f Distribution2D::SampleContinuous(const Point2f &u, Float *pdf) const
{
    Float pdfs[2];
    int v;
    Float d1 = pMarginal->SampleContinuous(u[1], &pdfs[1], &v);
    Float d0 = pConditionalV[v]->SampleContinuous(u[0], &pdfs[0]);
    *pdf = pdfs[0] * pdfs[1];
    return Point2f(d0, d1);
}

Float Distribution2D::Pdf(const Point2f &p) const
{
    int iu = Clamp(int(p[0] * pConditionalV[0]->Count()), 0,
                   pConditionalV[0]->Count() - 1);
    int iv = Clamp(int(p[1] * pMarginal->Count()), 0, pMarginal->Count() - 1);
    return pConditionalV[iv]->func[iu] / pMarginal->funcInt;
}

// 在半球内进行余弦加权采样的函数
Vector3f CosineSampleHemisphere(const Point2f &u)
{
    Point2f d = ConcentricSampleDisk(u);
    Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
    return Vector3f(d.x, d.y, z);
}

//计算半球余弦加权采样概率密度函数值
Float CosineHemispherePdf(Float cosTheta)
{
    return cosTheta * InvPi;
}

// 计算平衡性采样权重
Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

// 计算重要性采样权重
Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

}
