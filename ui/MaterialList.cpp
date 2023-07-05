//
//  MaterialList.cpp
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/7/5.
//

#include "MaterialList.h"

#ifdef _WIN32
    #define PATHSPLIT "\\"
#else
    #define PATHSPLIT "/"
#endif

std::string getResourcesDir()
{
    std::string path = __FILE__;
    printf("file path = %s\n", path.c_str());
    std::string::size_type posNX = path.find("GNXRayTracer");
    
    path = path.substr(0, posNX);
    
    path = path + "GNXRayTracer";
    path += PATHSPLIT;
    path += "Resources";
    path += PATHSPLIT;
    return path;
}

std::shared_ptr<Material> getSmileFacePlasticMaterial()
{
    std::unique_ptr<TextureMapping2D> map = std::make_unique<UVMapping2D>(1.f, 1.f, 0.f, 0.f);
    std::string filename = getResourcesDir() + "awesomeface.jpg";
    ImageWrap wrapMode = ImageWrap::Repeat;
    bool trilerp = false;
    float maxAniso = 8.f;
    float scale = 1.f;
    bool gamma = false; //
    std::shared_ptr<Texture<Spectrum>> Kt =
        std::make_shared<ImageTexture<RGBSpectrum, Spectrum>>(std::move(map), filename, trilerp, maxAniso, wrapMode, scale, gamma);

    std::shared_ptr<Texture<float>> plasticRoughness = std::make_shared<ConstantTexture<float>>(0.1f);
    std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);
    return std::make_shared<PlasticMaterial>(Kt, Kt, plasticRoughness, bumpMap, true);
}

std::shared_ptr<Material> getPurplePlasticMaterial()
{
    Spectrum purple; purple[0] = 0.35; purple[1] = 0.12; purple[2] = 0.48;
    std::shared_ptr<Texture<Spectrum>> plasticKd = std::make_shared<ConstantTexture<Spectrum>>(purple);
    std::shared_ptr<Texture<Spectrum>> plasticKr = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(1.f) - purple);
    std::shared_ptr<Texture<float >> plasticRoughness = std::make_shared<ConstantTexture<float >>(0.1f);
    std::shared_ptr<Texture<float >> bumpMap = std::make_shared<ConstantTexture<float >>(0.0f);
    return std::make_shared<PlasticMaterial >(plasticKd, plasticKr, plasticRoughness, bumpMap, true);
}

std::shared_ptr<Material> getYelloMetalMaterial()
{
    Spectrum eta; eta[0] = 0.2f; eta[1] = 0.2f; eta[2] = 0.8f;
    std::shared_ptr<Texture<Spectrum>> etaM = std::make_shared<ConstantTexture<Spectrum>>(eta);
    Spectrum k; k[0] = 0.11f; k[1] = 0.11f; k[2] = 0.11f;
    std::shared_ptr<Texture<Spectrum>> kM = std::make_shared<ConstantTexture<Spectrum>>(k);
    std::shared_ptr<Texture<float>> Roughness = std::make_shared<ConstantTexture<float>>(0.15f);
    std::shared_ptr<Texture<float>> RoughnessU = std::make_shared<ConstantTexture<float>>(0.15f);
    std::shared_ptr<Texture<float>> RoughnessV = std::make_shared<ConstantTexture<float>>(0.15f);
    std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);
    return     std::make_shared<MetalMaterial>(etaM, kM, Roughness, RoughnessU, RoughnessV, bumpMap, false);
}

std::shared_ptr<Material> getWhiteGlassMaterial()
{
    Spectrum c1; c1[0] = 0.98f; c1[1] = 0.98f; c1[2] = 0.98f;
    std::shared_ptr<Texture<Spectrum>> Kr = std::make_shared<ConstantTexture<Spectrum>>(c1);
    Spectrum c2; c2[0] = 0.98f; c2[1] = 0.98f; c2[2] = 0.98f;
    std::shared_ptr<Texture<Spectrum>> Kt = std::make_shared<ConstantTexture<Spectrum>>(c2);
    std::shared_ptr<Texture<float>> index = std::make_shared<ConstantTexture<float>>(1.5f);
    std::shared_ptr<Texture<float>> RoughnessU = std::make_shared<ConstantTexture<float>>(0.1f);
    std::shared_ptr<Texture<float>> RoughnessV = std::make_shared<ConstantTexture<float>>(0.1f);
    std::shared_ptr<Texture<float>> bumpMap = std::make_shared<ConstantTexture<float>>(0.0f);
    return     std::make_shared<GlassMaterial>(Kr, Kt,
        RoughnessU, RoughnessV, index, bumpMap, false);
}
