#ifndef PBR_CORE_SAMPLER_H
#define PBR_CORE_SAMPLER_H

// core/sampler.h*
#include "Geometry.h"
#include "RNG.h"
#include "StringPrint.h"
#include <inttypes.h>

namespace pbr
{

// Sampler Declarations
class Sampler
{
  public:
    // Sampler Interface
    virtual ~Sampler();
    Sampler(int64_t samplesPerPixel);
    virtual void StartPixel(const Point2i &p);
    virtual Float Get1D() = 0;
    virtual Point2f Get2D() = 0;
    CameraSample GetCameraSample(const Point2i &pRaster);
    void Request1DArray(int n);
    void Request2DArray(int n);
    virtual int RoundCount(int n) const { return n; }
    const Float *Get1DArray(int n);
    const Point2f *Get2DArray(int n);
    virtual bool StartNextSample();
    virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
    virtual bool SetSampleNumber(int64_t sampleNum);
    std::string StateString() const
    {
      return StringPrintf("(%d,%d), sample %" PRId64, currentPixel.x,
                          currentPixel.y, currentPixelSampleIndex);
    }
    int64_t CurrentSampleNumber() const { return currentPixelSampleIndex; }

    // Sampler Public Data
    const int64_t samplesPerPixel;

  protected:
    // Sampler Protected Data
    Point2i currentPixel;
    int64_t currentPixelSampleIndex;
    std::vector<int> samples1DArraySizes, samples2DArraySizes;
    std::vector<std::vector<Float>> sampleArray1D;
    std::vector<std::vector<Point2f>> sampleArray2D;

  private:
    // Sampler Private Data
    size_t array1DOffset, array2DOffset;
};

class PixelSampler : public Sampler {
  public:
    // PixelSampler Public Methods
    PixelSampler(int64_t samplesPerPixel, int nSampledDimensions);
    bool StartNextSample();
    bool SetSampleNumber(int64_t);
    Float Get1D();
    Point2f Get2D();

  protected:
    // PixelSampler Protected Data
    std::vector<std::vector<Float>> samples1D;
    std::vector<std::vector<Point2f>> samples2D;
    int current1DDimension = 0, current2DDimension = 0;
    RNG rng;
};

class GlobalSampler : public Sampler {
  public:
    // GlobalSampler Public Methods
    bool StartNextSample();
    void StartPixel(const Point2i &);
    bool SetSampleNumber(int64_t sampleNum);
    Float Get1D();
    Point2f Get2D();
    GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) {}
    virtual int64_t GetIndexForSample(int64_t sampleNum) const = 0;
    virtual Float SampleDimension(int64_t index, int dimension) const = 0;

  private:
    // GlobalSampler Private Data
    int dimension;
    int64_t intervalSampleIndex;
    static const int arrayStartDim = 5;
    int arrayEndDim;
};

inline void ClockRandomInit()
{
    srand((unsigned)time(NULL));
}

inline double getClockRandom()
{
    return rand() / (RAND_MAX + 1.0);
}

// ClockRandSampler Declarations
class ClockRandSampler : public GlobalSampler {
public:
    // HaltonSampler Public Methods
    ClockRandSampler(int samplesPerPixel = 16, const Bounds2i& sampleBounds = Bounds2i(Point2i(0, 0), Point2i(100, 100))) :GlobalSampler(samplesPerPixel) {
        ClockRandomInit();
    }
    std::unique_ptr<Sampler> Clone(int seed) {
        return std::unique_ptr<Sampler>(new ClockRandSampler(*this));
    }
    int64_t GetIndexForSample(int64_t sampleNum) const {
        return 0;
    }
    float SampleDimension(int64_t index, int dimension) const {
        return getClockRandom();
    }
};

}  // namespace pbr

#endif  // PBR_CORE_SAMPLER_H
