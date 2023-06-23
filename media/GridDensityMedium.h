
#ifndef PBR_MEDIA_GRID_H
#define PBR_MEDIA_GRID_H

// media/grid.h*
#include "core/Medium.h"
#include "core/Transform.h"

namespace pbr
{

static inline uint64_t densityBytes = 0;

// GridDensityMedium Declarations
class GridDensityMedium : public Medium
{
public:
    // GridDensityMedium Public Methods
    GridDensityMedium(const Spectrum &sigma_a, const Spectrum &sigma_s, Float g,
                      int nx, int ny, int nz, const Transform &mediumToWorld,
                      const Float *d)
        : sigma_a(sigma_a),
          sigma_s(sigma_s),
          g(g),
          nx(nx),
          ny(ny),
          nz(nz),
          WorldToMedium(Inverse(mediumToWorld)),
          density(new Float[nx * ny * nz]) {
        densityBytes += nx * ny * nz * sizeof(Float);
        memcpy((Float *)density.get(), d, sizeof(Float) * nx * ny * nz);
        // Precompute values for Monte Carlo sampling of _GridDensityMedium_
        sigma_t = (sigma_a + sigma_s)[0];
//        if (Spectrum(sigma_t) != sigma_a + sigma_s)
//            Error(
//                "GridDensityMedium requires a spectrally uniform attenuation "
//                "coefficient!");
        Float maxDensity = 0;
        for (int i = 0; i < nx * ny * nz; ++i)
            maxDensity = std::max(maxDensity, density[i]);
        invMaxDensity = 1 / maxDensity;
    }

    Float Density(const Point3f &p) const;
    Float D(const Point3i &p) const {
        Bounds3i sampleBounds(Point3i(0, 0, 0), Point3i(nx, ny, nz));
        if (!InsideExclusive(p, sampleBounds)) return 0;
        return density[(p.z * ny + p.y) * nx + p.x];
    }
    Spectrum Sample(const Ray &ray, Sampler &sampler, MemoryArena &arena,
                    MediumInteraction *mi) const;
    Spectrum Tr(const Ray &ray, Sampler &sampler) const;

private:
    // GridDensityMedium Private Data
    const Spectrum sigma_a, sigma_s;
    const Float g;
    const int nx, ny, nz;
    const Transform WorldToMedium;
    std::unique_ptr<Float[]> density;
    Float sigma_t;
    Float invMaxDensity;
};

}  // namespace pbr

#endif  // PBR_MEDIA_GRID_H
