

// textures/imagemap.cpp*
#include "ImageTexture.h"
#include "thirdparty/stb/stb_image.h"

namespace pbr
{

template <typename Tmemory, typename Treturn>
std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>>
    ImageTexture<Tmemory, Treturn>::mTextures;

// gamma 解码只在这里做一次：
//   1) stb 的 ldr→hdr 转换固定关掉，统一拿到 8bit 原始值 v/255；
//   2) 颜色贴图（srgb=true）在这里做标准 sRGB 解码；
//   3) 法线/粗糙度这类数据贴图传 srgb=false，保持原始值；
//   4) .hdr 本身是线性浮点，不做解码。
// 下游的 convertIn 只负责 scale，不再参与 gamma，避免同一个纹理被解码两次。
static RGBSpectrum *loadImage(const std::string &filename, Point2i &resolution, bool srgb)
{
    if (filename.empty())
        return nullptr;

    stbi_ldr_to_hdr_gamma(1.0f);

    int imageWidth, imageHeight, fileChannels;
    float *data = stbi_loadf(filename.c_str(), &imageWidth, &imageHeight,
                             &fileChannels, 3); // 强制 3 通道，避免灰度/双通道图越界
    if (!data)
        return nullptr;

    bool decodeSRGB = srgb && !stbi_is_hdr(filename.c_str());
    decodeSRGB = false;

    RGBSpectrum *data_s = new RGBSpectrum[imageWidth * imageHeight];
    for (int j = 0; j < imageHeight; ++j)
    {
        for (int i = 0; i < imageWidth; ++i)
        {
            const int base = (i + j * imageWidth) * 3;
            RGBSpectrum r;
            for (int k = 0; k < 3; ++k)
            {
                const Float v = data[base + k];
                r[k] = decodeSRGB ? GammaCorrect(v) : v;
            }
            data_s[i + j * imageWidth] = r;
        }
    }
    resolution.x = imageWidth;
    resolution.y = imageHeight;
    stbi_image_free(data);
    return data_s;
}

template <typename Tmemory, typename Treturn>
ImageTexture<Tmemory, Treturn>::ImageTexture(
    std::unique_ptr<TextureMapping2D> mapping, const std::string &filename,
    bool doTrilinear, Float maxAniso, ImageWrap wrapMode, Float scale,
    bool gamma)
    : mMapping(std::move(mapping))
{
    mMipmap = GetTexture(filename, doTrilinear, maxAniso, wrapMode, scale, gamma);
}

template <typename Tmemory, typename Treturn>
MIPMap<Tmemory> *ImageTexture<Tmemory, Treturn>::GetTexture(
    const std::string &filename, bool doTrilinear, Float maxAniso,
    ImageWrap wrap, Float scale, bool gamma)
{
    // Return _MIPMap_ from texture cache if present
    TexInfo texInfo(filename, doTrilinear, maxAniso, wrap, scale, gamma);
    if (mTextures.find(texInfo) != mTextures.end())
        return mTextures[texInfo].get();

    // Create _MIPMap_ for _filename_
    Point2i resolution;
    std::unique_ptr<RGBSpectrum[]> texels(loadImage(filename, resolution, gamma));
    if (!texels)
    {
        resolution.x = resolution.y = 1;
        RGBSpectrum *rgb = new RGBSpectrum[1];
        *rgb = RGBSpectrum(0.5f);
        texels.reset(rgb);
    }

    // 上下颠倒，使得（0，0）点在图像的左下角
//    for (int y = 0; y < resolution.y / 2; ++y)
//    {
//        for (int x = 0; x < resolution.x; ++x)
//        {
//            int o1 = y * resolution.x + x;
//            int o2 = (resolution.y - 1 - y) * resolution.x + x;
//            std::swap(texels[o1], texels[o2]);
//        }
//    }

    MIPMap<Tmemory> *mipmap = nullptr;
    if (texels)
    {
        // Convert texels to type _Tmemory_ and create _MIPMap_
        std::unique_ptr<Tmemory[]> convertedTexels(
            new Tmemory[resolution.x * resolution.y]);
        for (int i = 0; i < resolution.x * resolution.y; ++i)
            convertIn(texels[i], &convertedTexels[i], scale);
        mipmap = new MIPMap<Tmemory>(resolution, convertedTexels.get(),
                                     doTrilinear, maxAniso, wrap);
    }
    else
    {
        // Create one-valued _MIPMap_
        Tmemory oneVal = scale;
        mipmap = new MIPMap<Tmemory>(Point2i(1, 1), &oneVal);
    }
    mTextures[texInfo].reset(mipmap);
    return mipmap;
}

template class ImageTexture<Float, Float>;
template class ImageTexture<RGBSpectrum, Spectrum>;

} // namespace pbr
