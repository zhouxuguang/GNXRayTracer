cmake_minimum_required(VERSION 3.5)

project(GNXRayTracer VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY 
        XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_property)

set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(CORE_FILES 
core/GNXRayTracer.h
core/Geometry.h 
core/Geometry.cpp
core/Interaction.h
core/Interaction.cpp
core/Primitive.h
core/Primitive.cpp
core/Shape.h
core/Shape.cpp
core/Transform.h 
core/Transform.cpp 
core/UnitTest.h
core/UnitTest.cpp
)
source_group("core" FILES ${CORE_FILES})

set(SHAPE_FILES 
shape/Sphere.h
shape/Sphere.cpp
shape/Triangle.h
shape/Triangle.cpp
)
source_group("shape" FILES ${SHAPE_FILES})

set(ACCELERATOR_FILES 
accelerator/BVHAccel.h
accelerator/BVHAccel.cpp
)
source_group("accelerator" FILES ${ACCELERATOR_FILES})

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
             ${SHAPE_FILES}
             ${ACCELERATOR_FILES}
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
endif (APPLE)

file(GLOB Icons ${CMAKE_CURRENT_SOURCE_DIR}/Icons/*.png)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${Icons} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/Icons/)

install(TARGETS GNXRayTracer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
