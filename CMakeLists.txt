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

# --- FastNoiseLite ---
CPMAddPackage(
    NAME FastNoiseLite
    GITHUB_REPOSITORY Auburn/FastNoiseLite
    GIT_TAG master
    DOWNLOAD_ONLY YES
)
if (FastNoiseLite_ADDED)
    add_library(FastNoiseLite INTERFACE)
    target_include_directories(FastNoiseLite INTERFACE ${FastNoiseLite_SOURCE_DIR}/Cpp)
    add_library(FastNoiseLite::FastNoiseLite ALIAS FastNoiseLite)
endif()
list(APPEND LIBS FastNoiseLite::FastNoiseLite)

# --- Lua ---
CPMAddPackage(
    NAME lua
    VERSION 5.4.6
    URL "https://www.lua.org/ftp/lua-5.4.6.tar.gz"
    DOWNLOAD_ONLY YES
)
if(lua_ADDED)
    file(GLOB LUA_SOURCES "${lua_SOURCE_DIR}/src/*.c")
    list(REMOVE_ITEM LUA_SOURCES "${lua_SOURCE_DIR}/src/lua.c" "${lua_SOURCE_DIR}/src/luac.c")
    # Create static library
    add_library(lua STATIC ${LUA_SOURCES})
    # Export headers
    target_include_directories(lua PUBLIC "${lua_SOURCE_DIR}/src")
endif()

list(APPEND LIBS lua)

# --- sol2 ---
CPMAddPackage("gh:ThePhD/sol2#v3.3.1")
list(APPEND LIBS sol2)

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
    "Light.h"
    "Chunk.h"
    "Constants.h"
    "Chunk.cpp"
    "Player.h"
    "World.h"
    "World.cpp"
    "InputHandler.h"
    "InputHandler.cpp"
    "UIRenderer.h"
    "UIRenderer.cpp"
    "BlockData.h"
    "BlockRegistry.h"
    "ModManager.h"
    "ModManager.cpp"
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

# --- Copy MODS folder ---
add_custom_target(CopyMods
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/mods"
    "$<TARGET_FILE_DIR:OpenGL3DGame>/mods/"
)
add_dependencies(OpenGL3DGame CopyMods)

if(MSVC)
    # Force PDB generation
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

if(MSVC)
    target_compile_options(OpenGL3DGame PRIVATE
        /O2
        /fp:fast
        /arch:AVX2
    )
else()
    target_compile_options(OpenGL3DGame PRIVATE
        -O3
        -ffast-math
        -mavx2
    )
endif()