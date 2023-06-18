

#ifndef PBR_CORE_MIPMAP_INCLUDE_H
#define PBR_CORE_MIPMAP_INCLUDE_H

// core/mipmap.h*
#include "Spectrum.h"
#include "Texture.h"
//#include "stats.h"
//#include "parallel.h"

namespace pbr 
{

static uint64_t nEWALookups = 0;
static uint64_t nTrilerpLookups = 0;
static uint64_t mipMapMemory = 0;

// MIPMap Helper Declarations
enum class ImageWrap { Repeat, Black, Clamp };

struct ResampleWeight {
    int firstTexel;
    Float weight[4];
};

// MIPMap Declarations
template <typename T>
class MIPMap 
{
public:
    // MIPMap Public Methods
    MIPMap(const Point2i &resolution, const T *data, bool doTri = false,
           Float maxAniso = 8.f, ImageWrap wrapMode = ImageWrap::Repeat);
    int Width() const { return resolution[0]; }
    int Height() const { return resolution[1]; }
    int Levels() const { return (int)pyramid.size(); }
    const T &Texel(int level, int s, int t) const;
    T Lookup(const Point2f &st, Float width = 0.f) const;
    T Lookup(const Point2f &st, Vector2f dstdx, Vector2f dstdy) const;

private:
    // MIPMap Private Methods
    std::unique_ptr<ResampleWeight[]> resampleWeights(int oldRes, int newRes)
    {
        CHECK_GE(newRes, oldRes);
        std::unique_ptr<ResampleWeight[]> wt(new ResampleWeight[newRes]);
        Float filterwidth = 2.f;
        for (int i = 0; i < newRes; ++i) {
            // Compute image resampling weights for _i_th texel
            Float center = (i + .5f) * oldRes / newRes;
            wt[i].firstTexel = std::floor((center - filterwidth) + 0.5f);
            for (int j = 0; j < 4; ++j) {
                Float pos = wt[i].firstTexel + j + .5f;
                wt[i].weight[j] = Lanczos((pos - center) / filterwidth);
            }

            // Normalize filter weights for texel resampling
            Float invSumWts = 1 / (wt[i].weight[0] + wt[i].weight[1] +
                                   wt[i].weight[2] + wt[i].weight[3]);
            for (int j = 0; j < 4; ++j) wt[i].weight[j] *= invSumWts;
        }
        return wt;
    }
    
    Float clamp(Float v) { return Clamp(v, 0.f, Infinity); }
    RGBSpectrum clamp(const RGBSpectrum &v) { return v.Clamp(0.f, Infinity); }
    SampledSpectrum clamp(const SampledSpectrum &v)
    {
        return v.Clamp(0.f, Infinity);
    }
    
    T triangle(int level, const Point2f &st) const;
    T EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const;

    // MIPMap Private Data
    const bool doTrilinear;
    const Float maxAnisotropy;
    const ImageWrap wrapMode;
    Point2i resolution;
    std::vector<std::unique_ptr<BlockedArray<T>>> pyramid;   //存储每一层的图像
    static constexpr int WeightLUTSize = 128;
    static Float weightLut[WeightLUTSize];
};

// MIPMap Method Definitions
template <typename T>
MIPMap<T>::MIPMap(const Point2i &res, const T *img, bool doTrilinear,
                  Float maxAnisotropy, ImageWrap wrapMode)
    : doTrilinear(doTrilinear),
      maxAnisotropy(maxAnisotropy),
      wrapMode(wrapMode),
      resolution(res) 
{
    //ProfilePhase _(Prof::MIPMapCreation);

    //如果不是2的n次方，则扩展到2的n次方
    std::unique_ptr<T[]> resampledImage = nullptr;
    if (!IsPowerOf2(resolution[0]) || !IsPowerOf2(resolution[1])) 
    {
        // Resample image to power-of-two resolution
        Point2i resPow2(RoundUpPow2(resolution[0]), RoundUpPow2(resolution[1]));
        /*LOG(INFO) << "Resampling MIPMap from " << resolution << " to " <<
            resPow2 << ". Ratio= " << (Float(resPow2.x * resPow2.y) /
                                       Float(resolution.x * resolution.y));*/
        // Resample image in $s$ direction
        std::unique_ptr<ResampleWeight[]> sWeights = resampleWeights(resolution[0], resPow2[0]);
        resampledImage.reset(new T[resPow2[0] * resPow2[1]]);

        // Apply _sWeights_ to zoom in $s$ direction  这里可以采用并行化的策略
        for (int64_t t = 0; t < resolution[1]; ++t)
        {
            for (int s = 0; s < resPow2[0]; ++s) {
                // Compute texel $(s,t)$ in $s$-zoomed image
                resampledImage[t * resPow2[0] + s] = 0.f;
                for (int j = 0; j < 4; ++j) {
                    int origS = sWeights[s].firstTexel + j;
                    if (wrapMode == ImageWrap::Repeat)
                        origS = Mod(origS, resolution[0]);
                    else if (wrapMode == ImageWrap::Clamp)
                        origS = Clamp(origS, 0, resolution[0] - 1);
                    if (origS >= 0 && origS < (int)resolution[0])
                        resampledImage[t * resPow2[0] + s] +=
                        sWeights[s].weight[j] *
                        img[t * resolution[0] + origS];
                }
            }
        }

        // Resample image in $t$ direction
        std::unique_ptr<ResampleWeight[]> tWeights = resampleWeights(resolution[1], resPow2[1]);
        /*std::vector<T *> resampleBufs;
        int nThreads = MaxThreadIndex();
        for (int i = 0; i < nThreads; ++i)
            resampleBufs.push_back(new T[resPow2[1]]);*/
        
        T* workData = new T[resPow2[1]];
        for (int64_t s = 0; s < resPow2[0]; ++s) 
        {
            for (int t = 0; t < resPow2[1]; ++t) 
            {
                workData[t] = 0.f;
                for (int j = 0; j < 4; ++j) {
                    int offset = tWeights[t].firstTexel + j;
                    if (wrapMode == ImageWrap::Repeat)
                        offset = Mod(offset, resolution[1]);
                    else if (wrapMode == ImageWrap::Clamp)
                        offset = Clamp(offset, 0, (int)resolution[1] - 1);
                    if (offset >= 0 && offset < (int)resolution[1])
                        workData[t] += tWeights[t].weight[j] *
                        resampledImage[offset * resPow2[0] + s];
                }
            }
            for (int t = 0; t < resPow2[1]; ++t)
                resampledImage[t * resPow2[0] + s] = clamp(workData[t]);

        }
        delete[]workData;

        //for (auto ptr : resampleBufs) delete[] ptr;
        resolution = resPow2;
    }
    // 计算层级数目
    int nLevels = 1 + Log2Int(std::max(resolution[0], resolution[1]));
    pyramid.resize(nLevels);

    // Initialize most detailed level of MIPMap
    pyramid[0].reset(
        new BlockedArray<T>(resolution[0], resolution[1], resampledImage ? resampledImage.get() : img));
    for (int i = 1; i < nLevels; ++i) 
    {
        // Initialize $i$th MIPMap level from $i-1$st level
        int sRes = std::max(1, pyramid[i - 1]->uSize() / 2);
        int tRes = std::max(1, pyramid[i - 1]->vSize() / 2);
        pyramid[i].reset(new BlockedArray<T>(sRes, tRes));

        // Filter four texels from finer level of pyramid ,这里也可以用并行加速
        for (int t = 0; t < tRes; t++)
        {
            for (int s = 0; s < sRes; ++s)
            {
                (*pyramid[i])(s, t) =
                    .25f * (Texel(i - 1, 2 * s, 2 * t) +
                        Texel(i - 1, 2 * s + 1, 2 * t) +
                        Texel(i - 1, 2 * s, 2 * t + 1) +
                        Texel(i - 1, 2 * s + 1, 2 * t + 1));
            }
        }
    }

    // Initialize EWA filter weights if needed
    if (weightLut[0] == 0.) {
        for (int i = 0; i < WeightLUTSize; ++i) {
            Float alpha = 2;
            Float r2 = Float(i) / Float(WeightLUTSize - 1);
            weightLut[i] = std::exp(-alpha * r2) - std::exp(-alpha);
        }
    }
    mipMapMemory += (4 * resolution[0] * resolution[1] * sizeof(T)) / 3;
}

template <typename T>
const T &MIPMap<T>::Texel(int level, int s, int t) const 
{
    CHECK_LT(level, pyramid.size());
    const BlockedArray<T> &l = *pyramid[level];
    // Compute texel $(s,t)$ accounting for boundary conditions
    switch (wrapMode) {
    case ImageWrap::Repeat:
        s = Mod(s, l.uSize());
        t = Mod(t, l.vSize());
        break;
    case ImageWrap::Clamp:
        s = Clamp(s, 0, l.uSize() - 1);
        t = Clamp(t, 0, l.vSize() - 1);
        break;
    case ImageWrap::Black: {
        static const T black = 0.f;
        if (s < 0 || s >= (int)l.uSize() || t < 0 || t >= (int)l.vSize())
            return black;
        break;
    }
    }
    return l(s, t);
}

template <typename T>
T MIPMap<T>::Lookup(const Point2f &st, Float width) const 
{
    ++nTrilerpLookups;
    //ProfilePhase p(Prof::TexFiltTrilerp);
    // Compute MIPMap level for trilinear filtering
    Float level = Levels() - 1 + Log2(std::max(width, (Float)1e-8));

    // Perform trilinear interpolation at appropriate MIPMap level
    if (level < 0)
        return triangle(0, st);
    else if (level >= Levels() - 1)
        return Texel(Levels() - 1, 0, 0);
    else {
        int iLevel = std::floor(level);
        Float delta = level - iLevel;
        return Lerp(delta, triangle(iLevel, st), triangle(iLevel + 1, st));
    }
}

template <typename T>
T MIPMap<T>::triangle(int level, const Point2f &st) const 
{
    level = Clamp(level, 0, Levels() - 1);
    Float s = st[0] * pyramid[level]->uSize() - 0.5f;
    Float t = st[1] * pyramid[level]->vSize() - 0.5f;
    int s0 = std::floor(s), t0 = std::floor(t);
    Float ds = s - s0, dt = t - t0;
    return (1 - ds) * (1 - dt) * Texel(level, s0, t0) +
           (1 - ds) * dt * Texel(level, s0, t0 + 1) +
           ds * (1 - dt) * Texel(level, s0 + 1, t0) +
           ds * dt * Texel(level, s0 + 1, t0 + 1);
}

template <typename T>
T MIPMap<T>::Lookup(const Point2f &st, Vector2f dst0, Vector2f dst1) const 
{
    if (doTrilinear) {
        Float width = std::max(std::max(std::abs(dst0[0]), std::abs(dst0[1])),
                               std::max(std::abs(dst1[0]), std::abs(dst1[1])));
        return Lookup(st, width);
    }
    ++nEWALookups;
    //ProfilePhase p(Prof::TexFiltEWA);
    // Compute ellipse minor and major axes
    if (dst0.LengthSquared() < dst1.LengthSquared()) std::swap(dst0, dst1);
    Float majorLength = dst0.Length();
    Float minorLength = dst1.Length();

    // Clamp ellipse eccentricity if too large
    if (minorLength * maxAnisotropy < majorLength && minorLength > 0) {
        Float scale = majorLength / (minorLength * maxAnisotropy);
        dst1 *= scale;
        minorLength *= scale;
    }
    if (minorLength == 0) return triangle(0, st);

    // Choose level of detail for EWA lookup and perform EWA filtering
    Float lod = std::max((Float)0, Levels() - (Float)1 + Log2(minorLength));
    int ilod = std::floor(lod);
    return Lerp(lod - ilod, EWA(ilod, st, dst0, dst1),
                EWA(ilod + 1, st, dst0, dst1));
}

template <typename T>
T MIPMap<T>::EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const 
{
    if (level >= Levels()) return Texel(Levels() - 1, 0, 0);
    // Convert EWA coordinates to appropriate scale for level
    st[0] = st[0] * pyramid[level]->uSize() - 0.5f;
    st[1] = st[1] * pyramid[level]->vSize() - 0.5f;
    dst0[0] *= pyramid[level]->uSize();
    dst0[1] *= pyramid[level]->vSize();
    dst1[0] *= pyramid[level]->uSize();
    dst1[1] *= pyramid[level]->vSize();

    // Compute ellipse coefficients to bound EWA filter region
    Float A = dst0[1] * dst0[1] + dst1[1] * dst1[1] + 1;
    Float B = -2 * (dst0[0] * dst0[1] + dst1[0] * dst1[1]);
    Float C = dst0[0] * dst0[0] + dst1[0] * dst1[0] + 1;
    Float invF = 1 / (A * C - B * B * 0.25f);
    A *= invF;
    B *= invF;
    C *= invF;

    // Compute the ellipse's $(s,t)$ bounding box in texture space
    Float det = -B * B + 4 * A * C;
    Float invDet = 1 / det;
    Float uSqrt = std::sqrt(det * C), vSqrt = std::sqrt(A * det);
    int s0 = std::ceil(st[0] - 2 * invDet * uSqrt);
    int s1 = std::floor(st[0] + 2 * invDet * uSqrt);
    int t0 = std::ceil(st[1] - 2 * invDet * vSqrt);
    int t1 = std::floor(st[1] + 2 * invDet * vSqrt);

    // Scan over ellipse bound and compute quadratic equation
    T sum(0.f);
    Float sumWts = 0;
    for (int it = t0; it <= t1; ++it) {
        Float tt = it - st[1];
        for (int is = s0; is <= s1; ++is) {
            Float ss = is - st[0];
            // Compute squared radius and filter texel if inside ellipse
            Float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
            if (r2 < 1) {
                int index =
                    std::min((int)(r2 * WeightLUTSize), WeightLUTSize - 1);
                Float weight = weightLut[index];
                sum += Texel(level, is, it) * weight;
                sumWts += weight;
            }
        }
    }
    return sum / sumWts;
}

template <typename T>
Float MIPMap<T>::weightLut[WeightLUTSize];

}  // namespace pbr

#endif  // PBR_CORE_MIPMAP_INCLUDE_H
