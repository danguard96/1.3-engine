cmake_minimum_required(VERSION 3.19)

project(VulkanEngine VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable parallel builds
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include directories for bootstrapped dependencies
include_directories(deps deps/src external)

# Add ImGui include directory
include_directories(deps/src/lightweightvk/third-party/deps/src/imgui)

# Add subdirectories for bootstrapped libraries
add_subdirectory(deps/src/glm)

# LightweightVK setup
set(LIGHTWEIGHTVK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/lightweightvk)
include_directories(${LIGHTWEIGHTVK_SOURCE_DIR}/include)

# Disable Tracy to avoid build issues
set(LVK_WITH_TRACY OFF CACHE BOOL "Disable Tracy profiler")
set(LVK_WITH_TRACY_GPU OFF CACHE BOOL "Disable Tracy GPU profiler")

# Enable ImGui support
set(LVK_WITH_IMGUI ON CACHE BOOL "Enable ImGui support")

# Configure ktx like the cookbook
set(KTX_FEATURE_DOC            OFF CACHE BOOL "")
set(KTX_FEATURE_GL_UPLOAD      OFF CACHE BOOL "")
set(KTX_FEATURE_JNI            OFF CACHE BOOL "")
set(KTX_FEATURE_KTX1            ON CACHE BOOL "")   # Re-enable for LightweightVK samples
set(KTX_FEATURE_KTX2            ON CACHE BOOL "")   # Re-enable for LightweightVK samples
set(KTX_FEATURE_LOADTEST_APPS  OFF CACHE BOOL "")
set(KTX_FEATURE_STATIC_LIBRARY  ON CACHE BOOL "")
set(KTX_FEATURE_TESTS          OFF CACHE BOOL "")
set(KTX_FEATURE_TOOLS          OFF CACHE BOOL "")
set(KTX_FEATURE_VK_UPLOAD      OFF CACHE BOOL "")

# KTX configuration (working)
# Help CMake find bash for ktx
if(WIN32)
    find_program(BASH_EXECUTABLE bash PATHS "C:/Program Files/Git/bin" "C:/Program Files/Git/usr/bin")
    if(BASH_EXECUTABLE)
        message(STATUS "Found bash: ${BASH_EXECUTABLE}")
    endif()
    
    # Additional KTX configuration for MinGW
    set(KTX_FEATURE_LOADTEST_APPS OFF CACHE BOOL "")
    set(KTX_FEATURE_TOOLS OFF CACHE BOOL "")
    set(KTX_FEATURE_TESTS OFF CACHE BOOL "")
endif()

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
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "")

add_subdirectory(deps/src/assimp)
if(APPLE)
  target_compile_options(assimp PRIVATE -Wno-deprecated-declarations)
endif()

# Set folder property for assimp
set_property(TARGET assimp PROPERTY FOLDER "third-party")

# Additional assimp-specific flags for FBX parser issues
if(NOT MSVC)
    target_compile_options(assimp PRIVATE 
        -Wno-unused-variable
        -Wno-unused-function
        -Wno-unused-parameter
    )
    # Only add dangling-reference flags for GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(assimp PRIVATE 
            -Wno-dangling-reference 
            -Wno-error=dangling-reference
        )
    endif()
endif()

# Handle zlib on Windows like the cookbook
if(WIN32)
  if(TARGET zlibstatic)
    set_property(TARGET zlibstatic PROPERTY FOLDER "third-party")
  endif()
  if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "third-party")
  endif()
endif()

# Disable warnings in third-party code
if(MSVC)
    target_compile_options(assimp PRIVATE /wd4267)
else()
    # Apply comprehensive warning suppression for assimp
    target_compile_options(assimp PRIVATE 
        $<$<COMPILE_LANGUAGE:C>:-Wno-error -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-sign-compare>
    )
    # Add GCC-specific C++ warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(assimp PRIVATE 
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-dangling-reference -Wno-error=dangling-reference>
        )
    endif()
endif()

# Add global compiler flags to suppress warnings for all targets
if(NOT MSVC)
    add_compile_options(-Wno-unused-variable -Wno-unused-function -Wno-unused-parameter)
    # Additional flags for dangling reference issues (GCC only)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wno-dangling-reference -Wno-error=dangling-reference)
    endif()
endif()

# macOS-specific fixes
if(APPLE)
    # Suppress additional warnings that can cause build failures
    add_compile_options(-Wno-deprecated-non-prototype -Wno-macro-redefined)
endif()

# MinGW compatibility (working)

# Main engine sources
file(GLOB_RECURSE EngineSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Optimize build performance
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")

# Create the main executable
add_executable(VulkanEngine ${EngineSources})

# Link libraries - use the same approach as cookbook
target_link_libraries(VulkanEngine PRIVATE 
    glm::glm
    glfw 
    Vulkan::Vulkan 
    assimp
    LVKLibrary
    LVKVulkan
)

# Set target properties for optimization
set_target_properties(VulkanEngine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compile definitions
target_compile_definitions(VulkanEngine PRIVATE 
    # STB_IMAGE_IMPLEMENTATION removed - defined in MeshComponent.cpp only
)

# Include directories
target_include_directories(VulkanEngine PRIVATE 
    "src"
    "deps/src/stb"
    ${LIGHTWEIGHTVK_SOURCE_DIR}/include
)

# Compile features
target_compile_features(VulkanEngine PRIVATE cxx_std_20)

# Enable optimizations for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(VulkanEngine PRIVATE -O3 -DNDEBUG)
endif()

set(LVK_WITH_GLFW ON CACHE BOOL "Enable GLFW window support")
target_compile_definitions(VulkanEngine PUBLIC LVK_WITH_GLFW=1)

# Shader compilation removed - using runtime loading

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