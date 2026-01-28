cmake_minimum_required(VERSION 3.20)

project("OpenGL3DGame")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# --- 1. SETUP CPM ---
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)


# --- 2. CONFIGURATION ---
set(LIBS)


# --- 3. DEPENDENCIES ---

# --- GLFW ---
CPMAddPackage("gh:glfw/glfw#9352d8f")
list(APPEND LIBS glfw)

# --- GLEW ---
CPMAddPackage(
    NAME glew
    URL https://github.com/nigels-com/glew/releases/download/glew-2.3.0/glew-2.3.0.zip
    SOURCE_SUBDIR build/cmake
)
list(APPEND LIBS glew_s)
add_definitions(-DGLEW_STATIC)

# --- SOIL2 ---
CPMAddPackage("gh:SpartanJ/SOIL2#e9ae69d")
list(APPEND LIBS soil2)

# --- GLM ---
CPMAddPackage("gh:g-truc/glm#e7970a8")
list(APPEND LIBS glm::glm)

# --- OpenGL ---
find_package(OpenGL REQUIRED)


# --- 4. EXECUTABLE CONFIGURATION ---
add_executable(OpenGL3DGame 
    "OpenGL3DGame.cpp"
    "libs.h"
    "Shader.h"
    "Texture.h"
    "Material.h"
    "Vertex.h"
    "Mesh.h"
    "Primitives.h"
    "Game.h"
    "Game.cpp"
    "Camera.h"
    "Model.h"
)

target_include_directories(OpenGL3DGame PRIVATE
    "${glew_SOURCE_DIR}/include"
    "${glew-cmake_SOURCE_DIR}"
    
    ${SOIL2_SOURCE_DIR}/src
)

target_compile_features(OpenGL3DGame PRIVATE cxx_std_20)

# --- 5. LINKING & INCLUDE ---
target_link_libraries(OpenGL3DGame PRIVATE ${LIBS} OpenGL::GL)

# --- 6. FINAL FIXES ---

add_custom_target(CopyResources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:OpenGL3DGame>/resources/"
)
add_dependencies(OpenGL3DGame CopyResources)

add_custom_target(CopyImages
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    "${CMAKE_CURRENT_SOURCE_DIR}/images" 
    "$<TARGET_FILE_DIR:OpenGL3DGame>/images/"
)
add_dependencies(OpenGL3DGame CopyImages)