//
//  UnitTest.cpp
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/5/1.
//

#include "UnitTest.h"
#include "Geometry.h"
#include "Transform.h"

void testGeometry()
{
    pbr::Vector3f vec1(0, 1, 0);
    pbr::Vector3f vec2(1, 0, 0);
    pbr::Float value = pbr::Dot(vec1, vec2);
    printf("");
}

void testTransform()
{
    pbr::Vector3f vec1(10, 100, 1000);
    pbr::Transform transform = pbr::Translate(vec1);
    printf("");
}
