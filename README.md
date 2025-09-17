# Vulkan Engine 1.3

A modern, lightweight Vulkan-based graphics engine built with C++20 and designed for learning and experimentation with modern graphics programming techniques.

## Features

- **Modern C++20**: Uses latest C++ features and best practices
- **Vulkan 1.3**: Full Vulkan API support with LightweightVK wrapper
- **Component-Based Architecture**: Entity-Component system for flexible game object management
- **Cross-Platform**: Windows, Linux, and macOS support
- **Dependency Management**: Automated dependency management with corporateshark/bootstrapping
- **Asset Pipeline**: Support for OBJ models, textures, and YAML scene files
- **Shader System**: GLSL shader compilation with SPIR-V
- **Scene Management**: YAML-based scene configuration and loading
- **Math Utilities**: Comprehensive math library with GLM integration
- **File Utilities**: Cross-platform file system operations

## Dependencies

This project uses the [bootstrapping](https://github.com/corporateshark/bootstrapping) dependency management system to handle external libraries.

### Required Dependencies

- **Vulkan SDK 1.3+** - Graphics API
- **Python 3** - For bootstrapping script
- **Git** - For cloning repositories
- **CMake 3.30+** - Build system
- **C++20 Compiler** - GCC 11+, Clang 14+, or MSVC 2022+

### External Libraries (Managed by Bootstrap)

- **[LightweightVK](https://github.com/corporateshark/lightweightvk)** - Vulkan wrapper library
- **GLM** - Mathematics library
- **GLFW** - Window management
- **spdlog** - Logging library
- **Dear ImGui** - Immediate mode GUI
- **tiny_obj_loader** - OBJ file loader
- **stb_image** - Image loading
- **yaml-cpp** - YAML parsing and generation

## Quick Setup

**Option 1: Automated Setup (Recommended)**
```bash
git clone <repository-url>
cd vulkan-engine-1.3
chmod +x setup.sh
./setup.sh
```

**Option 2: Manual Setup**
```bash
git clone <repository-url>
cd vulkan-engine-1.3
cd deps && python3 bootstrap.py && cd ..
python3 scripts/copy_headers.py
mkdir build && cd build
cmake .. -G "Unix Makefiles"
make
```

## Project Structure

```
vulkan-engine-1.3/
├── src/                    # Engine source code
│   ├── Core/              # Core engine systems
│   │   ├── Engine.h/cpp   # Main engine class
│   │   ├── Window.h/cpp   # Window management
│   │   ├── Application.h/cpp # Application layer
│   │   └── Logger.h/cpp   # Logging system
│   ├── Rendering/         # Rendering system
│   │   └── VulkanRenderer.h/cpp # Vulkan renderer
│   ├── Scene/             # Scene management
│   │   ├── Scene.h/cpp    # Scene class
│   │   ├── Entity.h/cpp   # Entity system
│   │   └── Components.h/cpp # Component system
│   ├── Utils/             # Utility classes
│   │   ├── MathUtils.h/cpp # Math utilities
│   │   └── FileUtils.h/cpp # File operations
│   └── main.cpp           # Entry point
├── assets/                # Game assets
│   ├── models/            # 3D models
│   ├── textures/          # 2D textures
│   └── scenes/            # Scene files
├── shaders/               # GLSL shaders
│   ├── basic.vert         # Basic vertex shader
│   └── basic.frag         # Basic fragment shader
├── deps/                  # Dependency management
│   ├── bootstrap.py       # Bootstrap script
│   └── bootstrap.json     # Dependency configuration
├── cmake/                 # CMake modules
│   └── Shaders.cmake      # Shader compilation
├── scripts/               # Build and utility scripts
│   ├── setup.sh           # Setup script
│   ├── build.sh           # Build script
│   ├── run.sh             # Run script
│   ├── clean.sh           # Clean script
│   └── copy_headers.py    # Header copying script
├── external/              # Header-only libraries
├── CMakeLists.txt         # Main CMake configuration
└── README.md              # This file
```

## Build Commands

### Setup (First time only)
```bash
./setup.sh
```

### Build
```bash
./scripts/build.sh
# or
cd build && make
```

### Run
```bash
./scripts/run.sh
# or
cd build && ./bin/VulkanEngine
```

### Clean
```bash
./scripts/clean.sh
```

## Usage

### Basic Engine Usage

```cpp
#include "Core/Engine.h"
#include "Core/Application.h"

int main() {
    // Initialize the engine
    auto engine = std::make_unique<VulkanEngine::Engine>();
    engine->Initialize();
    
    // Create and run the application
    auto app = std::make_unique<VulkanEngine::Application>(engine.get());
    app->Initialize();
    app->Run();
    
    // Cleanup
    app->Shutdown();
    engine->Shutdown();
    
    return 0;
}
```

### Creating Entities and Components

```cpp
// Create an entity
auto entity = std::make_shared<VulkanEngine::Entity>();
entity->SetName("MyEntity");

// Add components
auto transform = entity->AddComponent<VulkanEngine::TransformComponent>();
transform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
transform->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
transform->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

auto mesh = entity->AddComponent<VulkanEngine::MeshComponent>();
mesh->LoadFromFile("assets/models/cube.obj");

// Add to scene
scene->AddEntity(entity);
```

### Scene Configuration (YAML)

```yaml
scene:
  name: "My Scene"
  entities:
    - name: "Cube"
      transform:
        position: { x: 0.0, y: 0.0, z: 0.0 }
        rotation: { x: 0.0, y: 0.0, z: 0.0 }
        scale: { x: 1.0, y: 1.0, z: 1.0 }
      mesh:
        file: "assets/models/cube.obj"
```

## Architecture

### Core Systems

1. **Engine**: Main engine class that manages the overall application lifecycle
2. **Window**: GLFW-based window management with input handling
3. **VulkanRenderer**: LightweightVK-based Vulkan rendering system
4. **Application**: High-level application layer that manages scenes and game logic

### Scene System

1. **Scene**: Container for entities and scene-specific data
2. **Entity**: Game object that can hold multiple components
3. **Component**: Modular functionality that can be added to entities
   - TransformComponent: Position, rotation, scale
   - MeshComponent: 3D model rendering
   - CameraComponent: Camera functionality

### Rendering Pipeline

1. **VulkanRenderer**: Manages Vulkan context and rendering
2. **LightweightVK**: Simplified Vulkan API wrapper
3. **Shader System**: GLSL to SPIR-V compilation
4. **Asset Loading**: OBJ models, textures, and materials

## Adding New Dependencies

To add a new dependency:

1. Edit `deps/bootstrap.json` and add the library configuration
2. Run `cd deps && python3 bootstrap.py && cd ..` to download the new dependency
3. Update `CMakeLists.txt` to include the new library
4. If it's a header-only library, update `scripts/copy_headers.py`

## Building

The project uses CMake as the build system. After running the setup script, you can build normally with CMake.

### CMake Configuration

- **C++20 Standard**: Uses modern C++ features
- **Vulkan Integration**: Requires Vulkan SDK
- **Precompiled Headers**: Faster compilation with `precomp.h`
- **Shader Compilation**: Automatic GLSL to SPIR-V compilation
- **Asset Copying**: Assets are copied to build directory

## Troubleshooting

### Common Issues

1. **Vulkan SDK not found**: Make sure Vulkan SDK is installed and `glslc` is in your PATH
2. **CMake version**: Requires CMake 3.30 or higher
3. **C++20 compiler**: Make sure you have a C++20 compatible compiler
4. **Dependencies**: If bootstrap fails, check your internet connection and Git installation

### Debug Mode

To enable debug mode:
```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [LightweightVK](https://github.com/corporateshark/lightweightvk) for the Vulkan wrapper
- [Bootstrap](https://github.com/corporateshark/bootstrapping) for dependency management
- [GLM](https://github.com/g-truc/glm) for mathematics
- [GLFW](https://github.com/glfw/glfw) for window management
- [spdlog](https://github.com/gabime/spdlog) for logging
- [Dear ImGui](https://github.com/ocornut/imgui) for immediate mode GUI
