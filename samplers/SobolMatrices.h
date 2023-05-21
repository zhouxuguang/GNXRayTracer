
#ifndef PBR_CORE_SOBOLMATRICES_H
#define PBR_CORE_SOBOLMATRICES_H

// core/sobolmatrices.h*
#include "core/GNXRayTracer.h"

namespace pbr
{

// Sobol Matrix Declarations
static constexpr int NumSobolDimensions = 1024;
static constexpr int SobolMatrixSize = 52;
extern const uint32_t SobolMatrices32[NumSobolDimensions * SobolMatrixSize];
extern const uint64_t SobolMatrices64[NumSobolDimensions * SobolMatrixSize];
extern const uint64_t VdCSobolMatrices[][SobolMatrixSize];
extern const uint64_t VdCSobolMatricesInv[][SobolMatrixSize];

}  // namespace pbr

#endif  // PBR_CORE_SOBOLMATRICES_H
