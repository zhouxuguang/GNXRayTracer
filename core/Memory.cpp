

// core/memory.cpp*
#include "Memory.h"

namespace pbr
{

// Memory Allocation Functions
void *AllocAligned(size_t size)
{
#if defined(PBR_HAVE__ALIGNED_MALLOC)
    return _aligned_malloc(size, PBR_L1_CACHE_LINE_SIZE);
#elif defined(PBR_HAVE_POSIX_MEMALIGN)
    void *ptr;
    if (posix_memalign(&ptr, PBR_L1_CACHE_LINE_SIZE, size) != 0) ptr = nullptr;
    return ptr;
#else
    return memalign(PBR_L1_CACHE_LINE_SIZE, size);
#endif
}

void FreeAligned(void *ptr)
{
    if (!ptr) return;
#if defined(PBR_HAVE__ALIGNED_MALLOC)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

}  // namespace pbr
