cmake_minimum_required(VERSION 3.5)

project(GNXRayTracer VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY 
        XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_property)


if (WIN32)
    find_package(OpenMP)
endif (WIN32)

include_directories(thirdparty/libomp/include)
link_directories(thirdparty/libomp/lib/mac)

set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g")



set(MINIFB_BUILD_EXAMPLES OFF CACHE BOOL "Build minifb example programs" FORCE)
set(MINIFB_BUILD_VERSION_INFO OFF CACHE BOOL "Build version info utility" FORCE)
add_subdirectory(thirdparty/minifb)

set(CORE_FILES
core/GNXRayTracer.h
core/Camera.h 
core/Camera.cpp
core/Filter.h
core/Filter.cpp
core/Geometry.h 
core/Geometry.cpp
core/Integrator.h
core/Integrator.cpp
core/Interpolation.h
core/Interpolation.cpp
core/Interaction.h
core/Interaction.cpp
core/Light.h
core/Light.cpp
core/LightDistribution.h
core/LightDistribution.cpp
core/Material.h
core/Material.cpp
core/Medium.h
core/Medium.cpp
core/Memory.h
core/Memory.cpp
core/MicroFacet.h
core/MicroFacet.cpp
core/MIPMap.h
core/MIPMap.cpp
core/Primitive.h
core/Primitive.cpp
core/Quaternion.h
core/Quaternion.cpp
core/RNG.h
core/Reflection.h
core/Reflection.cpp
core/SubReflection.h
core/SubReflection.cpp
core/Sampler.h
core/Sampler.cpp
core/Sampling.h
core/Sampling.cpp
core/Scene.h
core/Scene.cpp
core/Shape.h
core/Shape.cpp
core/Spectrum.h
core/Spectrum.cpp
core/StringPrint.h
core/Texture.h
core/Texture.cpp
core/Transform.h 
core/Transform.cpp 
core/UnitTest.h
core/UnitTest.cpp
core/SceneLoader.h
core/SceneLoader.cpp
)
source_group("core" FILES ${CORE_FILES})

set(CAMERA_FILES 
camera/Orthographic.h 
camera/Orthographic.cpp
camera/Perspective.h 
camera/Perspective.cpp
)
source_group("camera" FILES ${CAMERA_FILES})

set(FILTERS_FILES
filters/GaussianFilter.h
filters/GaussianFilter.cpp)
source_group("filters" FILES ${FILTERS_FILES})

set(SHAPE_FILES 
shape/Sphere.h
shape/Sphere.cpp
shape/Triangle.h
shape/Triangle.cpp
shape/plyRead.h
)
source_group("shape" FILES ${SHAPE_FILES})

set(ACCELERATOR_FILES 
accelerator/BVHAccel.h
accelerator/BVHAccel.cpp
)
source_group("accelerator" FILES ${ACCELERATOR_FILES})

set(LIGHTS_FILES 
lights/PointLight.h
lights/PointLight.cpp
lights/DiffuseAreaLight.h
lights/DiffuseAreaLight.cpp
lights/DistantLight.h
lights/DistantLight.cpp
lights/InfiniteAreaLight.h
lights/InfiniteAreaLight.cpp
lights/SkyBoxLight.h
lights/SkyBoxLight.cpp
lights/SpotLight.h
lights/SpotLight.cpp
)
source_group("lights" FILES ${LIGHTS_FILES})

set(SAMPLERS_FILES 
samplers/HaltonSampler.h 
samplers/HaltonSampler.cpp
samplers/LowDiscrepancy.h
samplers/LowDiscrepancy.cpp
samplers/SobolMatrices.h
samplers/SobolMatrices.cpp
)
source_group("samplers" FILES ${SAMPLERS_FILES})

set(MATERIALS_FILES
materials/DisneyMaterial.h
materials/DisneyMaterial.cpp
materials/GlassMaterial.h
materials/GlassMaterial.cpp
materials/MatteMaterial.h 
materials/MatteMaterial.cpp
materials/MetalMaterial.h
materials/MetalMaterial.cpp
materials/MirrorMaterial.h
materials/MirrorMaterial.cpp
materials/PlasticMaterial.h
materials/PlasticMaterial.cpp
)
source_group("materials" FILES ${MATERIALS_FILES})

set(TEXTURES_FILES 
textures/ConstantTexture.h 
textures/ConstantTexture.cpp
textures/ImageTexture.h
textures/ImageTexture.cpp
)
source_group("textures" FILES ${TEXTURES_FILES})

set(INTEGRATORS_FILES 
integrators/DirectLightingIntegrator.h
integrators/DirectLightingIntegrator.cpp
integrators/PathIntegrator.h
integrators/PathIntegrator.cpp
integrators/WhittedIntegrator.h
integrators/WhittedIntegrator.cpp
integrators/VolPathIntegrator.h
integrators/VolPathIntegrator.cpp
)
source_group("integrators" FILES ${INTEGRATORS_FILES})

set(MEDIA_FILES 
media/HomogeneousMedium.h
media/HomogeneousMedium.cpp
media/GridDensityMedium.h
media/GridDensityMedium.cpp
)
source_group("media" FILES ${MEDIA_FILES})

set(UI_FILES
ui/Renderer.h
ui/Renderer.cpp
ui/main.cpp
)
source_group("ui" FILES ${UI_FILES})

set(STB_FILES 
thirdparty/stb/stb_image.h
thirdparty/stb/stb_image_write.h
)
source_group("stb" FILES ${STB_FILES})

set(TINYXML2_FILES
thirdparty/tinyxml2/tinyxml2.h
thirdparty/tinyxml2/tinyxml2.cpp
)
source_group("tinyxml2" FILES ${TINYXML2_FILES})

add_executable(GNXRayTracer
             ${CORE_FILES}
             ${CAMERA_FILES}
             ${FILTERS_FILES}
             ${SHAPE_FILES}
             ${ACCELERATOR_FILES}
             ${SAMPLERS_FILES}
             ${TEXTURES_FILES}
             ${MATERIALS_FILES}
             ${LIGHTS_FILES}
             ${INTEGRATORS_FILES}
             ${MEDIA_FILES}
             ${UI_FILES}
             ${STB_FILES}
             ${TINYXML2_FILES}
        )

target_link_libraries(GNXRayTracer PRIVATE minifb)

set_target_properties(GNXRayTracer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
# ui/main.cpp uses standard main(); without Qt there is no qtmain.lib to supply WinMain.
if(MSVC)
    set_property(TARGET GNXRayTracer APPEND_STRING PROPERTY LINK_FLAGS " /ENTRY:mainCRTStartup")
endif()

if (APPLE)

    set_xcode_property(GNXRayTracer CLANG_CXX_LANGUAGE_STANDARD "c++17")
    set_xcode_property(GNXRayTracer CLANG_CXX_LIBRARY "libc++")
    set_xcode_property(GNXRayTracer GCC_ENABLE_CPP_EXCEPTIONS NO)
    set_xcode_property(GNXRayTracer GCC_ENABLE_CPP_RTTI NO)
elseif(WIN32)
    if(MSVC AND MSVC_VERSION GREATER_EQUAL 1913)
        #target_compile_options(GNXRayTracer ${visibility} "-Zc:__cplusplus" "-permissive-")
        #target_compile_options(GNXRayTracer ${visibility} "/utf-8 /Zc:__cplusplus")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /Zc:__cplusplus /permissive-")
    endif()
endif (APPLE)


install(TARGETS GNXRayTracer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
