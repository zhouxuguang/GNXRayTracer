cmake_minimum_required(VERSION 3.5)

project(GNXRayTracer VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(PROJECT_SOURCES
       main.cpp
       mainwindow.cpp
       mainwindow.h
)

set(CORE_FILES 
core/GNXRayTracer.h
core/Geometry.h 
core/Geometry.cpp
core/Primitive.h
core/Primitive.cpp
core/Shape.h
core/Shape.cpp
core/Transform.h 
core/Transform.cpp 
core/UnitTest.h
core/UnitTest.cpp
)

set(SHAPE_FILES 
shape/Sphere.h
shape/Sphere.cpp
)

add_executable(GNXRayTracer
             ${PROJECT_SOURCES}
             ${CORE_FILES}
             ${SHAPE_FILES}
        )

target_link_libraries(GNXRayTracer PRIVATE "-framework CoreVideo" "-framework QuartzCore")
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

install(TARGETS GNXRayTracer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
