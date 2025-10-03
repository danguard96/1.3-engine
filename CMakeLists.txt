cmake_minimum_required(VERSION 3.19)

project(VulkanEngine VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include directories for bootstrapped dependencies
include_directories(deps)
include_directories(deps/src)
include_directories(external)

# Remove ImGui from our deps since LightweightVK provides it
set(CMAKE_DISABLE_FIND_PACKAGE_ImGui TRUE)

# Add subdirectories for bootstrapped libraries
add_subdirectory(deps/src/glm)
add_subdirectory(deps/src/spdlog)
add_subdirectory(deps/src/yaml-cpp)

# LightweightVK setup
set(LIGHTWEIGHTVK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/lightweightvk)
include_directories(${LIGHTWEIGHTVK_SOURCE_DIR}/include)

# Disable Tracy to avoid build issues
set(LVK_WITH_TRACY OFF CACHE BOOL "Disable Tracy profiler")
set(LVK_WITH_TRACY_GPU OFF CACHE BOOL "Disable Tracy GPU profiler")

add_subdirectory(${LIGHTWEIGHTVK_SOURCE_DIR})

# Assimp setup - use cookbook approach
set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "")

add_subdirectory(deps/src/assimp)
if(APPLE)
  target_compile_options(assimp PRIVATE -Wno-deprecated-declarations)
endif()

# Set folder property for assimp
set_property(TARGET assimp PROPERTY FOLDER "third-party")

# Disable warnings in third-party code
if(MSVC)
    target_compile_options(assimp PRIVATE /wd4267)
else()
    # Only apply to C++ files, not C files
    target_compile_options(assimp PRIVATE 
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-dangling-reference>
    )
endif()

# Main engine sources
file(GLOB_RECURSE EngineSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Create the main executable
add_executable(VulkanEngine ${EngineSources} src/main.cpp)

# Link libraries - use the same approach as cookbook
target_link_libraries(VulkanEngine PRIVATE 
    glm::glm
    glfw 
    Vulkan::Vulkan 
    spdlog::spdlog 
    yaml-cpp
    assimp
    LVKLibrary
    LVKVulkan
)

# Compile definitions
target_compile_definitions(VulkanEngine PRIVATE 
    TINYOBJLOADER_IMPLEMENTATION
    STB_IMAGE_IMPLEMENTATION
)

# Include directories
target_include_directories(VulkanEngine PRIVATE 
    "src"
    "deps/src/tiny_obj_loader"
    "deps/src/stb"
    "deps/src/yaml-cpp/include"
    ${LIGHTWEIGHTVK_SOURCE_DIR}/include
)

# Compile features
target_compile_features(VulkanEngine PRIVATE cxx_std_20)

# Use LightweightVK's ImGui instead of our own to avoid version conflicts

# Precompiled headers
#target_precompile_headers(VulkanEngine PRIVATE "src/precomp.h")

set(LVK_WITH_GLFW ON CACHE BOOL "Enable GLFW window support")
target_compile_definitions(VulkanEngine PUBLIC LVK_WITH_GLFW=1)

# Shader compilation
include(cmake/Shaders.cmake)
file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
    "shaders/*.vert"
    "shaders/*.frag"
    "shaders/*.comp"
)

add_shaders(VulkanEngineShaders ${ShaderSources})
add_dependencies(VulkanEngine VulkanEngineShaders)

# Copy assets to build directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Copy shaders to build directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Set output directory
set_target_properties(VulkanEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Windows specific settings
if(WIN32)
    target_compile_definitions(VulkanEngine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(VulkanEngine PRIVATE DEBUG_MODE)
endif()