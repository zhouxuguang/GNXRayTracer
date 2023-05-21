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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(CORE_FILES 
core/GNXRayTracer.h
core/Camera.h 
core/Camera.cpp
core/Geometry.h 
core/Geometry.cpp
core/Integrator.h
core/Integrator.cpp
core/Interaction.h
core/Interaction.cpp
core/Light.h
core/Light.cpp
core/Material.h
core/Material.cpp
core/Memory.h
core/Memory.cpp
core/Primitive.h
core/Primitive.cpp
core/RNG.h
core/Reflection.h
core/Reflection.cpp
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
)
source_group("core" FILES ${CORE_FILES})

set(CAMERA_FILES 
camera/Orthographic.h 
camera/Orthographic.cpp
camera/Perspective.h 
camera/Perspective.cpp
)
source_group("camera" FILES ${CAMERA_FILES})

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
)
source_group("lights" FILES ${LIGHTS_FILES})

set(SAMPLERS_FILES 
samplers/Halton.h 
samplers/Halton.cpp
samplers/LowDiscrepancy.h
samplers/LowDiscrepancy.cpp
samplers/SobolMatrices.h
samplers/SobolMatrices.cpp
)
source_group("samplers" FILES ${SAMPLERS_FILES})

set(MATERIALS_FILES 
materials/Matte.h 
materials/Matte.cpp
)
source_group("materials" FILES ${MATERIALS_FILES})

set(TEXTURES_FILES 
textures/Constant.h 
textures/Constant.cpp
)
source_group("textures" FILES ${TEXTURES_FILES})

set(UI_FILES 
ui/FrameBuffer.h
ui/FrameBuffer.cpp
ui/DebugText.h
ui/DebugText.cpp
ui/DataTreeWidget.h
ui/DataTreeWidget.cpp
ui/RenderStatus.h
ui/RenderStatus.cpp
ui/IMAGraphicsView.h
ui/IMAGraphicsView.cpp
ui/DisplayWidget.h
ui/DisplayWidget.cpp
ui/RenderThread.h
ui/RenderThread.cpp
ui/InteractionDockWidget.h
ui/InteractionDockWidget.cpp
ui/MainWindow.h
ui/MainWindow.cpp
ui/main.cpp
)
source_group("ui" FILES ${UI_FILES})

add_executable(GNXRayTracer
             ${CORE_FILES}
             ${CAMERA_FILES}
             ${SHAPE_FILES}
             ${ACCELERATOR_FILES}
             ${SAMPLERS_FILES}
             ${TEXTURES_FILES}
             ${MATERIALS_FILES}
             ${LIGHTS_FILES}
             ${UI_FILES}
        )

#target_link_libraries(GNXRayTracer PRIVATE "-framework CoreVideo" "-framework QuartzCore")
target_link_libraries(GNXRayTracer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(GNXRayTracer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
set_target_properties(GNXRayTracer PROPERTIES AUTOUIC ON)
set_target_properties(GNXRayTracer PROPERTIES AUTOMOC ON)
set_target_properties(GNXRayTracer PROPERTIES AUTORCC ON)

if (APPLE)

    set_xcode_property(GNXRayTracer CLANG_CXX_LANGUAGE_STANDARD "c++17")
    set_xcode_property(GNXRayTracer CLANG_CXX_LIBRARY "libc++")
    set_xcode_property(GNXRayTracer GCC_ENABLE_CPP_EXCEPTIONS NO)
    set_xcode_property(GNXRayTracer GCC_ENABLE_CPP_RTTI NO)

    set (CMAKE_C_FLAGS "-Xclang -fopenmp")
    set (CMAKE_CXX_FLAGS "-Xclang -fopenmp")
    target_link_libraries(GNXRayTracer PRIVATE omp)
elseif(WIN32)
    target_link_libraries(GNXRayTracer PRIVATE OpenMP::OpenMP_CXX)
    if(MSVC AND MSVC_VERSION GREATER_EQUAL 1913)
        #target_compile_options(GNXRayTracer ${visibility} "-Zc:__cplusplus" "-permissive-")
        #target_compile_options(GNXRayTracer ${visibility} "/utf-8 /Zc:__cplusplus")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /Zc:__cplusplus /permissive-")
    endif()
endif (APPLE)

file(GLOB Icons ${CMAKE_CURRENT_SOURCE_DIR}/Icons/*.png)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${Icons} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/Icons/)

install(TARGETS GNXRayTracer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
