#ifndef GNX_CORE_RAYTRACER_H
#define GNX_CORE_RAYTRACER_H

// Global Include Files
#include <type_traits>
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>

#ifdef WIN32
#include <intrin.h>
#endif // WIN32


// debug-only checking.  not executed in NDEBUG mode.
#define DCHECK(condition) 
#define DCHECK_EQ(val1, val2) 
#define DCHECK_NE(val1, val2) 
#define DCHECK_LE(val1, val2) 
#define DCHECK_LT(val1, val2) 
#define DCHECK_GE(val1, val2) 
#define DCHECK_GT(val1, val2) 
#define DCHECK_NOTNULL(val) 
#define DCHECK_STREQ(str1, str2) 
#define DCHECK_STRCASEEQ(str1, str2) 
#define DCHECK_STRNE(str1, str2) 
#define DCHECK_STRCASENE(str1, str2) 

#define CHECK(condition) 
#define CHECK_EQ(val1, val2) 
#define CHECK_NE(val1, val2) 
#define CHECK_LE(val1, val2) 
#define CHECK_LT(val1, val2) 
#define CHECK_GE(val1, val2) 
#define CHECK_GT(val1, val2) 

// Global Macros
#define ALLOCA(TYPE, COUNT) (TYPE *) alloca((COUNT) * sizeof(TYPE))

namespace pbr {

// Global Forward Declarations
class Scene;
class Integrator;
class SamplerIntegrator;
template <typename T>
class Vector2;
template <typename T>
class Vector3;
template <typename T>
class Point3;
template <typename T>
class Point2;
template <typename T>
class Normal3;
class Ray;
class RayDifferential;
template <typename T>
class Bounds2;
template <typename T>
class Bounds3;
class Transform;
struct Interaction;
class SurfaceInteraction;
class Shape;
class Primitive;
class GeometricPrimitive;
class TransformedPrimitive;
template <int nSpectrumSamples>
class CoefficientSpectrum;
class RGBSpectrum;
class SampledSpectrum;
#ifdef PBR_SAMPLED_SPECTRUM
  typedef SampledSpectrum Spectrum;
#else
  typedef RGBSpectrum Spectrum;
#endif
class Camera;
struct CameraSample;
class ProjectiveCamera;
class Sampler;
class Filter;
class Film;
class FilmTile;
class BxDF;
class BRDF;
class BTDF;
class BSDF;
class Material;
template <typename T>
class Texture;
class Medium;
class MediumInteraction;
struct MediumInterface;
class BSSRDF;
class SeparableBSSRDF;
class TabulatedBSSRDF;
struct BSSRDFTable;
class Light;
class VisibilityTester;
class AreaLight;
struct Distribution1D;
class Distribution2D;
#ifdef FLOAT_AS_DOUBLE
  typedef double Float;
#else
  typedef float Float;
#endif  // FLOAT_AS_DOUBLE
class RNG;
class ProgressReporter;

#ifndef PBR_L1_CACHE_LINE_SIZE
  #define PBR_L1_CACHE_LINE_SIZE 64
#endif

#ifdef _WIN32
    #define PBR_HAVE__ALIGNED_MALLOC
#else
    #define PBR_HAVE_POSIX_MEMALIGN
#endif


// Global Constants

static constexpr Float MaxFloat = std::numeric_limits<Float>::max();
static constexpr Float Infinity = std::numeric_limits<Float>::infinity();

static constexpr Float MachineEpsilon = std::numeric_limits<Float>::epsilon() * 0.5;
static constexpr Float ShadowEpsilon = 0.0001f;
static constexpr Float Pi = 3.14159265358979323846;
static constexpr Float InvPi = 0.31830988618379067154;
static constexpr Float Inv2Pi = 0.15915494309189533577;
static constexpr Float Inv4Pi = 0.07957747154594766788;
static constexpr Float PiOver2 = 1.57079632679489661923;
static constexpr Float PiOver4 = 0.78539816339744830961;
static constexpr Float Sqrt2 = 1.41421356237309504880;

// Global Inline Functions
inline uint32_t FloatToBits(float f)
{
    uint32_t ui;
    memcpy(&ui, &f, sizeof(float));
    return ui;
}

inline float BitsToFloat(uint32_t ui)
{
    float f;
    memcpy(&f, &ui, sizeof(uint32_t));
    return f;
}

inline uint64_t FloatToBits(double f)
{
    uint64_t ui;
    memcpy(&ui, &f, sizeof(double));
    return ui;
}

inline double BitsToFloat(uint64_t ui)
{
    double f;
    memcpy(&f, &ui, sizeof(uint64_t));
    return f;
}

inline float NextFloatUp(float v)
{
    // Handle infinity and negative zero for _NextFloatUp()_
    if (std::isinf(v) && v > 0.) return v;
    if (v == -0.f) v = 0.f;

    // Advance _v_ to next higher float
    uint32_t ui = FloatToBits(v);
    if (v >= 0)
        ++ui;
    else
        --ui;
    return BitsToFloat(ui);
}

inline float NextFloatDown(float v)
{
    // Handle infinity and positive zero for _NextFloatDown()_
    if (std::isinf(v) && v < 0.) return v;
    if (v == 0.f) v = -0.f;
    uint32_t ui = FloatToBits(v);
    if (v > 0)
        --ui;
    else
        ++ui;
    return BitsToFloat(ui);
}

inline double NextFloatUp(double v, int delta = 1)
{
    if (std::isinf(v) && v > 0.) return v;
    if (v == -0.f) v = 0.f;
    uint64_t ui = FloatToBits(v);
    if (v >= 0.)
        ui += delta;
    else
        ui -= delta;
    return BitsToFloat(ui);
}

inline double NextFloatDown(double v, int delta = 1)
{
    if (std::isinf(v) && v < 0.) return v;
    if (v == 0.f) v = -0.f;
    uint64_t ui = FloatToBits(v);
    if (v > 0.)
        ui -= delta;
    else
        ui += delta;
    return BitsToFloat(ui);
}

template <typename T, typename U, typename V>
inline T Clamp(T val, U low, V high)
{
    if (val < low)
        return low;
    else if (val > high)
        return high;
    else
        return val;
}

template <typename T>
inline T Mod(T a, T b)
{
    T result = a - (a / b) * b;
    return (T)((result < 0) ? result + b : result);
}

template <>
inline Float Mod(Float a, Float b)
{
    return std::fmod(a, b);
}

inline Float Radians(Float deg) { return (Pi / 180) * deg; }

inline Float Degrees(Float rad) { return (180 / Pi) * rad; }

inline Float Log2(Float x) {
    const Float invLog2 = 1.442695040888963387004650940071;
    return std::log(x) * invLog2;
}

inline int Log2Int(uint32_t v) 
{
#if defined(_MSC_VER)
    unsigned long lz = 0;
    if (_BitScanReverse(&lz, v)) return lz;
    return 0;
#else
    return 31 - __builtin_clz(v);
#endif
}

inline int Log2Int(int32_t v) { return Log2Int((uint32_t)v); }

inline int Log2Int(uint64_t v) 
{
#if defined(_MSC_VER)
    unsigned long lz = 0;
#if defined(_WIN64)
    _BitScanReverse64(&lz, v);
#else
    if (_BitScanReverse(&lz, v >> 32))
        lz += 32;
    else
        _BitScanReverse(&lz, v & 0xffffffff);
#endif // _WIN64
    return lz;
#else  // _MSC_VER
    return 63 - __builtin_clzll(v);
#endif
}

inline int Log2Int(int64_t v) { return Log2Int((uint64_t)v); }

template <typename T>
inline constexpr bool IsPowerOf2(T v) {
    return v && !(v & (v - 1));
}

inline int32_t RoundUpPow2(int32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

inline int64_t RoundUpPow2(int64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return v + 1;
}

inline int CountTrailingZeros(uint32_t v)
{
#if defined(WIN32)
    unsigned long index;
    if (_BitScanForward(&index, v))
        return index;
    else
        return 32;
#else
    return __builtin_ctz(v);
#endif
}

template <typename Predicate>
int FindInterval(int size, const Predicate &pred) {
    int first = 0, len = size;
    while (len > 0) {
        int half = len >> 1, middle = first + half;
        // Bisect range based on value of _pred_ at _middle_
        if (pred(middle)) {
            first = middle + 1;
            len -= half + 1;
        } else
            len = half;
    }
    return Clamp(first - 1, 0, size - 2);
}

inline Float Lerp(Float t, Float v1, Float v2) { return (1 - t) * v1 + t * v2; }

inline Float gamma(int n)
{
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

}  // namespace pbrt

#endif  //
