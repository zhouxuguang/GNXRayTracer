#ifndef PBR_CORE_SAMPLING_JJS_INCLUDE_H
#define PBR_CORE_SAMPLING_JJS_INCLUDE_H


#include "Geometry.h"
#include "RNG.h"

namespace pbr
{


Point2f ConcentricSampleDisk(const Point2f &u);

template <typename T>
void Shuffle(T *samp, int count, int nDimensions, RNG &rng)
{
    for (int i = 0; i < count; ++i)
    {
        int other = i + rng.UniformUInt32(count - i);
        for (int j = 0; j < nDimensions; ++j)
        {
            std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
        }
    }
}


}

#endif



