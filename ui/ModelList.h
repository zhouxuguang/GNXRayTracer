//
//  ModelList.hpp
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/7/5.
//

#ifndef ModelList_hpp
#define ModelList_hpp

#include "core/Primitive.h"
#include "shape/Triangle.h"

using namespace pbr;

void AddFloor(std::vector<std::shared_ptr<Primitive>> &prims, std::shared_ptr<Material> material);

void AddModel(std::vector<std::shared_ptr<Primitive>> &prims, std::shared_ptr<Material> material);

void AddCornell(std::vector<std::shared_ptr<Primitive>> &prims,
                std::shared_ptr<Material> material1,
                std::shared_ptr<Material> material2,
                std::shared_ptr<Material> material3);

void AddAreaLight(std::vector<std::shared_ptr<Primitive>> &prims, std::vector<std::shared_ptr<Light>>& lights,
                  std::shared_ptr<Material> material);

void AddSpotLight(std::vector<std::shared_ptr<Light>>& lights);

void AddDistLight(std::vector<std::shared_ptr<Light>>& lights);

void AddSkyLight(std::vector<std::shared_ptr<Light>>& lights);

void AddInfLight(std::vector<std::shared_ptr<Light>>& lights);

#endif /* ModelList_hpp */
