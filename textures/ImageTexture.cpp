

// textures/imagemap.cpp*
#include "ImageTexture.h"
#include "3rd/stb_image.h"

namespace pbr
{

template <typename Tmemory, typename Treturn>
std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>>
    ImageTexture<Tmemory, Treturn>::textures;

static RGBSpectrum* loadImage(const std::string &filename, Point2i &resolution)
{
    float *data;
    RGBSpectrum *data_s;
    if (filename != "") {
        //texels = ReadImage(texmap, &resolution);
        int imageWidth, imageHeight, nrComponents;
        //stbi_set_flip_vertically_on_load(true);
        data = stbi_loadf(filename.c_str(), &imageWidth, &imageHeight, &nrComponents, 0);
        if (!data)return nullptr;
        data_s = new RGBSpectrum[imageWidth * imageHeight];
        for (int j = 0; j < imageHeight; j++) {
            for (int i = 0; i < imageWidth; i++) {
                RGBSpectrum r;
                r[0] = data[(i + j * imageWidth) * nrComponents + 0];
                r[1] = data[(i + j * imageWidth) * nrComponents + 1];
                r[2] = data[(i + j * imageWidth) * nrComponents + 2];
                data_s[i + j * imageWidth] = r;
            }
        }
        resolution.x = imageWidth;
        resolution.y = imageHeight;
        free(data);
        return data_s;
    }
    return nullptr;
}

// ImageTexture Method Definitions
template <typename Tmemory, typename Treturn>
ImageTexture<Tmemory, Treturn>::ImageTexture(
    std::unique_ptr<TextureMapping2D> mapping, const std::string &filename,
    bool doTrilinear, Float maxAniso, ImageWrap wrapMode, Float scale,
    bool gamma)
    : mapping(std::move(mapping))
{
    mipmap = GetTexture(filename, doTrilinear, maxAniso, wrapMode, scale, gamma);
}

template <typename Tmemory, typename Treturn>
MIPMap<Tmemory> *ImageTexture<Tmemory, Treturn>::GetTexture(
    const std::string &filename, bool doTrilinear, Float maxAniso,
    ImageWrap wrap, Float scale, bool gamma)
{
    // Return _MIPMap_ from texture cache if present
    TexInfo texInfo(filename, doTrilinear, maxAniso, wrap, scale, gamma);
    if (textures.find(texInfo) != textures.end())
        return textures[texInfo].get();

    // Create _MIPMap_ for _filename_
    //ProfilePhase _(Prof::TextureLoading);
    Point2i resolution;
    std::unique_ptr<RGBSpectrum[]> texels(loadImage(filename, resolution));
    if (!texels) {
//        Warning("Creating a constant grey texture to replace \"%s\".",
//                filename.c_str());
        resolution.x = resolution.y = 1;
        RGBSpectrum *rgb = new RGBSpectrum[1];
        *rgb = RGBSpectrum(0.5f);
        texels.reset(rgb);
    }

    // Flip image in y; texture coordinate space has (0,0) at the lower
    // left corner.
    for (int y = 0; y < resolution.y / 2; ++y)
        for (int x = 0; x < resolution.x; ++x) {
            int o1 = y * resolution.x + x;
            int o2 = (resolution.y - 1 - y) * resolution.x + x;
            std::swap(texels[o1], texels[o2]);
        }

    MIPMap<Tmemory> *mipmap = nullptr;
    if (texels) {
        // Convert texels to type _Tmemory_ and create _MIPMap_
        std::unique_ptr<Tmemory[]> convertedTexels(
            new Tmemory[resolution.x * resolution.y]);
        for (int i = 0; i < resolution.x * resolution.y; ++i)
            convertIn(texels[i], &convertedTexels[i], scale, gamma);
        mipmap = new MIPMap<Tmemory>(resolution, convertedTexels.get(),
                                     doTrilinear, maxAniso, wrap);
    } else {
        // Create one-valued _MIPMap_
        Tmemory oneVal = scale;
        mipmap = new MIPMap<Tmemory>(Point2i(1, 1), &oneVal);
    }
    textures[texInfo].reset(mipmap);
    return mipmap;
}

template class ImageTexture<Float, Float>;
template class ImageTexture<RGBSpectrum, Spectrum>;

}  // namespace pbrt
