cmake_minimum_required(VERSION 3.15)
project(Simplification)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)
find_package(ASSIMP REQUIRED)
# find_package(GLUT REQUIRED)
include_directories(
        ${GLUT_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIRS}
        "../OpenGL/Libraies/includes")
find_library(GLFW_LIB libglfw.3.dylib "../OpenGL/Libraies/libs")
find_library(ASSIMP_LIB libassimp.4.dylib ${ASSIMP_LIBRARY_DIRS})

add_executable(Simplification main.cpp glad.c simplification.cpp stb_image.cpp)
target_link_libraries(Simplification ${OPENGL_LIBRARY} ${GLFW_LIB} ${ASSIMP_LIB})
# add_executable(test test.cpp glad.c simplification.cpp)
# target_link_libraries(test ${OPENGL_LIBRARY} ${GLFW_LIB} ${ASSIMP_LIB})