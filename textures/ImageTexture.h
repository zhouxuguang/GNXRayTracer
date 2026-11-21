
#ifndef PBR_TEXTURES_IMAGEMAP_H
#define PBR_TEXTURES_IMAGEMAP_H

#include "core/Texture.h"
#include "core/MIPMap.h"
#include <map>

namespace pbr
{

// 纹理信息结构-元数据信息
struct TexInfo
{
    TexInfo(const std::string &f, bool dt, Float ma, ImageWrap wm, Float sc,
            bool gamma)
        : filename(f),
          doTrilinear(dt),
          maxAniso(ma),
          wrapMode(wm),
          scale(sc),
          gamma(gamma)
    {
    }
    std::string filename;
    bool doTrilinear;
    Float maxAniso;
    ImageWrap wrapMode;
    Float scale;
    bool gamma;
    bool operator<(const TexInfo &t2) const
    {
        if (filename != t2.filename)
            return filename < t2.filename;
        if (doTrilinear != t2.doTrilinear)
            return doTrilinear < t2.doTrilinear;
        if (maxAniso != t2.maxAniso)
            return maxAniso < t2.maxAniso;
        if (scale != t2.scale)
            return scale < t2.scale;
        if (gamma != t2.gamma)
            return !gamma;
        return wrapMode < t2.wrapMode;
    }
};

// 图像纹理
template <typename Tmemory, typename Treturn>
class ImageTexture : public Texture<Treturn>
{
public:
    ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma);

    static void ClearCache()
    {
        mTextures.erase(mTextures.begin(), mTextures.end());
    }

    Treturn Evaluate(const SurfaceInteraction &si) const
    {
        Vector2f dstdx, dstdy;
        Point2f st = mMapping->Map(si, &dstdx, &dstdy);
        Tmemory mem = mMipmap->Lookup(st, dstdx, dstdy);
        Treturn ret;
        convertOut(mem, &ret);
        return ret;
    }

private:
    static MIPMap<Tmemory> *GetTexture(const std::string &filename,
                                       bool doTrilinear, Float maxAniso,
                                       ImageWrap wm, Float scale, bool gamma);
    static void convertIn(const RGBSpectrum &from, RGBSpectrum *to, Float scale,
                          bool gamma)
    {
        for (int i = 0; i < RGBSpectrum::nSamples; ++i)
            (*to)[i] = scale * (gamma ? InverseGammaCorrect(from[i]) : from[i]);
    }
    static void convertIn(const RGBSpectrum &from, Float *to, Float scale,
                          bool gamma)
    {
        *to = scale * (gamma ? InverseGammaCorrect(from.y()) : from.y());
    }
    static void convertOut(const RGBSpectrum &from, Spectrum *to)
    {
        Float rgb[3];
        from.ToRGB(rgb);
        *to = Spectrum::FromRGB(rgb);
    }
    static void convertOut(Float from, Float *to)
    {
        *to = from;
    }

    std::unique_ptr<TextureMapping2D> mMapping;     //对应的映射
    MIPMap<Tmemory> *mMipmap;
    static std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> mTextures;   //纹理缓存
};

extern template class ImageTexture<Float, Float>;
extern template class ImageTexture<RGBSpectrum, Spectrum>;


} // namespace pbr

#endif // PBR_TEXTURES_IMAGEMAP_H
