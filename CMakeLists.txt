cmake_minimum_required(VERSION 3.30)

project(VulkanEngine VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include directories for bootstrapped dependencies
include_directories(deps)
include_directories(deps/src)
include_directories(external)

# Add subdirectories for bootstrapped libraries
add_subdirectory(deps/src/glm)
add_subdirectory(deps/src/glfw)
add_subdirectory(deps/src/spdlog)
add_subdirectory(deps/src/yaml-cpp)

# LightweightVK setup
set(LIGHTWEIGHTVK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/lightweightvk)
include_directories(${LIGHTWEIGHTVK_SOURCE_DIR}/include)

# ImGui setup
set(IMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui)
set(IMGUI_BACKENDS_DIR ${IMGUI_SOURCE_DIR}/backends)

set(IMGUI_CORE_SOURCES
    ${IMGUI_SOURCE_DIR}/imgui.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
)

# Main engine sources
file(GLOB_RECURSE EngineSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp"
)

# Create the main executable
add_executable(VulkanEngine ${EngineSources} ${IMGUI_CORE_SOURCES}
        src/main.cpp)

# Link libraries
target_link_libraries(VulkanEngine PRIVATE 
    glm 
    glfw 
    Vulkan::Vulkan 
    spdlog::spdlog 
    yaml-cpp
)

# Compile definitions
target_compile_definitions(VulkanEngine PRIVATE 
    TINYOBJLOADER_IMPLEMENTATION
    STB_IMAGE_IMPLEMENTATION
)

# Include directories
target_include_directories(VulkanEngine PRIVATE 
    "src"
    ${IMGUI_SOURCE_DIR}
    ${IMGUI_BACKENDS_DIR}
    "deps/src/tiny_obj_loader"
    "deps/src/stb"
    "deps/src/yaml-cpp/include"
    ${LIGHTWEIGHTVK_SOURCE_DIR}/include
)

# Compile features
target_compile_features(VulkanEngine PRIVATE cxx_std_20)

# Precompiled headers
#target_precompile_headers(VulkanEngine PRIVATE "src/precomp.h")

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
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Windows specific settings
if(WIN32)
    target_compile_definitions(VulkanEngine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(VulkanEngine PRIVATE DEBUG_MODE)
endif()
