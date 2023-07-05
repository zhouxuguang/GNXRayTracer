//
//  MaterialList.hpp
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/7/5.
//

#ifndef MaterialList_hpp
#define MaterialList_hpp

#include "core/Material.h"
#include "textures/ConstantTexture.h"
#include "textures/ImageTexture.h"

#include "materials/MatteMaterial.h"
#include "materials/MirrorMaterial.h"
#include "materials/PlasticMaterial.h"
#include "materials/MetalMaterial.h"
#include "materials/GlassMaterial.h"
#include "core/Texture.h"

#include <stdio.h>

using namespace pbr;

std::string getResourcesDir();

std::shared_ptr<Material> getSmileFacePlasticMaterial();

std::shared_ptr<Material> getPurplePlasticMaterial();

std::shared_ptr<Material> getYelloMetalMaterial();

std::shared_ptr<Material> getWhiteGlassMaterial();

#endif /* MaterialList_hpp */
