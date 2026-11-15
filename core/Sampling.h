#ifndef PBR_CORE_SAMPLING_JJS_INCLUDE_H
#define PBR_CORE_SAMPLING_JJS_INCLUDE_H


#include "Geometry.h"
#include "RNG.h"

namespace pbr
{

// Sampling Declarations
void StratifiedSample1D(Float *samples, int nsamples, RNG &rng,
                        bool jitter = true);
void StratifiedSample2D(Point2f *samples, int nx, int ny, RNG &rng,
                        bool jitter = true);
void LatinHypercube(Float *samples, int nSamples, int nDim, RNG &rng);

// 一维分布的类，它用于计算和采样一维函数的概率分布
struct Distribution1D
{
    // Distribution1D Public Methods
    Distribution1D(const Float *f, int n);

    int Count() const;

    Float SampleContinuous(Float u, Float *pdf, int *off = nullptr) const;

    int SampleDiscrete(Float u, Float *pdf = nullptr,
                       Float *uRemapped = nullptr) const;

    Float DiscretePDF(int index) const;

    // Distribution1D Public Data
    std::vector<Float> func, cdf;
    Float funcInt;
};

Point2f RejectionSampleDisk(RNG &rng);
Vector3f UniformSampleHemisphere(const Point2f &u);
Float UniformHemispherePdf();
Vector3f UniformSampleSphere(const Point2f &u);
Float UniformSpherePdf();
Vector3f UniformSampleCone(const Point2f &u, Float thetamax);
Vector3f UniformSampleCone(const Point2f &u, Float thetamax, const Vector3f &x,
                           const Vector3f &y, const Vector3f &z);
Float UniformConePdf(Float thetamax);

//单位圆盘上均匀采样
Point2f UniformSampleDisk(const Point2f &u);
Point2f ConcentricSampleDisk(const Point2f &u);
Point2f UniformSampleTriangle(const Point2f &u);


class Distribution2D
{
public:
    // Distribution2D Public Methods
    Distribution2D(const Float *data, int nu, int nv);

    Point2f SampleContinuous(const Point2f &u, Float *pdf) const;

    Float Pdf(const Point2f &p) const;

private:
    // Distribution2D Private Data
    std::vector<std::unique_ptr<Distribution1D>> pConditionalV;
    std::unique_ptr<Distribution1D> pMarginal;
};

// Sampling Inline Functions
template <typename T>
void Shuffle(T *samp, int count, int nDimensions, RNG &rng)
{
    for (int i = 0; i < count; ++i)
    {
        int other = i + rng.UniformUInt32(count - i);
        for (int j = 0; j < nDimensions; ++j)
            std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
    }
}

// 在余弦半球内进行采样的函数
Vector3f CosineSampleHemisphere(const Point2f &u);

//计算余弦半球采样概率密度函数值
Float CosineHemispherePdf(Float cosTheta);

// 计算平衡性采样权重
Float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf);

// 计算重要性采样权重
Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf);

}

#endif
