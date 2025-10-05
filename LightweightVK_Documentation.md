# LightweightVK Documentation

## Overview

LightweightVK is a modern, bindless-only Vulkan wrapper library designed for Vulkan 1.3+ with optional mesh shaders and ray tracing support. It's a deeply refactored fork of IGL (Instagram Graphics Library) that focuses on lean, agile, and bindless rendering.

## Key Design Principles

### 1. **Lean API**
- Minimalistic API without bloat (no `std::vector`, `std::unordered_map` in the API)
- Direct, efficient function calls
- Minimal overhead between application and Vulkan

### 2. **Bindless Architecture**
- Utilizes Vulkan 1.3+ dynamic rendering
- Descriptor indexing for modern bindless rendering
- Buffer device address features
- Ray tracing fully integrated with bindless design

### 3. **Agile Development**
- Playground for Vulkan experiments
- Quick exploration of ideas
- Rapid prototyping of Vulkan-based renderers

## Core Architecture

### Handle System

LightweightVK uses a sophisticated handle system for resource management:

```cpp
template<typename ObjectType>
class Handle final {
    uint32_t index_ = 0;  // Resource index
    uint32_t gen_ = 0;    // Generation counter
};
```

**Key Features:**
- **Type Safety**: Each handle type is specialized (e.g., `BufferHandle`, `TextureHandle`)
- **Non-Ref Counted**: Based on generation counters for efficient resource tracking
- **Validation**: Handles can be checked for validity with `valid()` and `empty()`
- **Memory Efficient**: 64-bit handles (32-bit index + 32-bit generation)

### Holder System

The `Holder<T>` class provides RAII (Resource Acquisition Is Initialization) for handles:

```cpp
template<typename HandleType>
class Holder final {
    lvk::IContext* ctx_ = nullptr;
    HandleType handle_ = {};
    
    // Automatic cleanup on destruction
    ~Holder() { lvk::destroy(ctx_, handle_); }
};
```

**Benefits:**
- **Automatic Cleanup**: Resources are automatically destroyed when Holder goes out of scope
- **Move Semantics**: Efficient resource transfer without copying
- **Exception Safety**: RAII ensures cleanup even during exceptions

## Core Classes

### IContext Interface

The main interface for all LightweightVK operations:

```cpp
class IContext {
public:
    // Command Buffer Management
    virtual ICommandBuffer& acquireCommandBuffer() = 0;
    virtual SubmitHandle submit(ICommandBuffer& commandBuffer, TextureHandle present = {}) = 0;
    virtual void wait(SubmitHandle handle) = 0;
    
    // Resource Creation
    virtual Holder<BufferHandle> createBuffer(const BufferDesc& desc, const char* debugName = nullptr, Result* outResult = nullptr) = 0;
    virtual Holder<TextureHandle> createTexture(const TextureDesc& desc, const char* debugName = nullptr, Result* outResult = nullptr) = 0;
    virtual Holder<RenderPipelineHandle> createRenderPipeline(const RenderPipelineDesc& desc, Result* outResult = nullptr) = 0;
    
    // Resource Management
    virtual void destroy(BufferHandle handle) = 0;
    virtual void destroy(TextureHandle handle) = 0;
    
    // Swapchain Management
    virtual TextureHandle getCurrentSwapchainTexture() = 0;
    virtual Format getSwapchainFormat() const = 0;
    virtual void recreateSwapchain(int newWidth, int newHeight) = 0;
};
```

### ICommandBuffer Interface

High-level command buffer interface for recording rendering commands:

```cpp
class ICommandBuffer {
public:
    // Rendering Control
    virtual void cmdBeginRendering(const lvk::RenderPass& renderPass, const lvk::Framebuffer& desc, const Dependencies& deps = {}) = 0;
    virtual void cmdEndRendering() = 0;
    
    // Pipeline Binding
    virtual void cmdBindRenderPipeline(lvk::RenderPipelineHandle handle) = 0;
    virtual void cmdBindComputePipeline(lvk::ComputePipelineHandle handle) = 0;
    
    // Drawing Commands
    virtual void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t baseInstance = 0) = 0;
    virtual void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t baseInstance = 0) = 0;
    
    // Resource Binding
    virtual void cmdBindVertexBuffer(uint32_t index, BufferHandle buffer, uint64_t bufferOffset = 0) = 0;
    virtual void cmdBindIndexBuffer(BufferHandle indexBuffer, IndexFormat indexFormat, uint64_t indexBufferOffset = 0) = 0;
    virtual void cmdPushConstants(const void* data, size_t size, size_t offset = 0) = 0;
};
```

## Resource Types

### Buffers

```cpp
struct BufferDesc {
    uint8_t usage = 0;                    // BufferUsageBits flags
    StorageType storage = StorageType_HostVisible;
    size_t size = 0;
    const void* data = nullptr;           // Initial data
    const char* debugName = "";
};

enum BufferUsageBits : uint8_t {
    BufferUsageBits_Index = 1 << 0,
    BufferUsageBits_Vertex = 1 << 1,
    BufferUsageBits_Uniform = 1 << 2,
    BufferUsageBits_Storage = 1 << 3,
    BufferUsageBits_Indirect = 1 << 4,
    BufferUsageBits_ShaderBindingTable = 1 << 5,
    BufferUsageBits_AccelStructBuildInputReadOnly = 1 << 6,
    BufferUsageBits_AccelStructStorage = 1 << 7
};
```

### Textures

```cpp
struct TextureDesc {
    TextureType type = TextureType_2D;
    Format format = Format_Invalid;
    Dimensions dimensions = {1, 1, 1};
    uint32_t numLayers = 1;
    uint32_t numSamples = 1;
    uint8_t usage = TextureUsageBits_Sampled;
    uint32_t numMipLevels = 1;
    StorageType storage = StorageType_Device;
    ComponentMapping swizzle = {};
    const void* data = nullptr;
    uint32_t dataNumMipLevels = 1;
    bool generateMipmaps = false;
    const char* debugName = "";
};
```

### Render Pipelines

```cpp
struct RenderPipelineDesc {
    Topology topology = Topology_Triangle;
    lvk::VertexInput vertexInput;
    
    ShaderModuleHandle smVert;
    ShaderModuleHandle smFrag;
    // ... other shader stages
    
    ColorAttachment color[LVK_MAX_COLOR_ATTACHMENTS] = {};
    Format depthFormat = Format_Invalid;
    Format stencilFormat = Format_Invalid;
    
    CullMode cullMode = lvk::CullMode_None;
    WindingMode frontFaceWinding = lvk::WindingMode_CCW;
    PolygonMode polygonMode = lvk::PolygonMode_Fill;
    
    uint32_t samplesCount = 1u;
    const char* debugName = "";
};
```

## Rendering Pipeline

### 1. **Context Creation**

```cpp
// Initialize GLFW window
GLFWwindow* window = lvk::initWindow("My App", width, height, false);

// Create Vulkan context
std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(
    window, width, height, lvk::ContextConfig{}
);
```

### 2. **Resource Creation**

```cpp
// Create vertex buffer
auto vertexBuffer = ctx->createBuffer({
    .usage = lvk::BufferUsageBits_Vertex,
    .storage = lvk::StorageType_HostVisible,
    .size = sizeof(vertices),
    .data = vertices,
    .debugName = "vertex_buffer"
});

// Create texture
auto texture = ctx->createTexture({
    .type = lvk::TextureType_2D,
    .format = lvk::Format_RGBA_UN8,
    .dimensions = {width, height, 1},
    .usage = lvk::TextureUsageBits_Sampled,
    .data = imageData,
    .debugName = "texture"
});
```

### 3. **Shader Creation**

```cpp
// Create shader modules
auto vertShader = ctx->createShaderModule({
    .stage = lvk::Stage_Vert,
    .data = vertexShaderSource,
    .debugName = "vertex_shader"
});

auto fragShader = ctx->createShaderModule({
    .stage = lvk::Stage_Frag,
    .data = fragmentShaderSource,
    .debugName = "fragment_shader"
});
```

### 4. **Pipeline Creation**

```cpp
// Create render pipeline
auto pipeline = ctx->createRenderPipeline({
    .smVert = vertShader,
    .smFrag = fragShader,
    .color = {{.format = ctx->getSwapchainFormat()}},
    .debugName = "main_pipeline"
});
```

### 5. **Rendering Loop**

```cpp
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    
    // Acquire command buffer
    lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
    
    // Begin rendering
    cmd.cmdBeginRendering(
        {.color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}}}},
        {.color = {{.texture = ctx->getCurrentSwapchainTexture()}}}
    );
    
    // Bind pipeline and resources
    cmd.cmdBindRenderPipeline(pipeline);
    cmd.cmdBindVertexBuffer(0, vertexBuffer);
    cmd.cmdBindIndexBuffer(indexBuffer, lvk::IndexFormat_UI32);
    
    // Draw
    cmd.cmdDrawIndexed(indexCount);
    
    // End rendering
    cmd.cmdEndRendering();
    
    // Submit and present
    ctx->submit(cmd, ctx->getCurrentSwapchainTexture());
}
```

## Advanced Features

### Ray Tracing Support

```cpp
// Create ray tracing pipeline
auto rtPipeline = ctx->createRayTracingPipeline({
    .smRayGen = rayGenShader,
    .smClosestHit = closestHitShader,
    .smMiss = missShader,
    .debugName = "ray_tracing_pipeline"
});

// Trace rays
cmd.cmdBindRayTracingPipeline(rtPipeline);
cmd.cmdTraceRays(width, height, depth);
```

### Mesh Shaders

```cpp
// Create mesh shader pipeline
auto meshPipeline = ctx->createRenderPipeline({
    .smTask = taskShader,
    .smMesh = meshShader,
    .smFrag = fragmentShader,
    .debugName = "mesh_shader_pipeline"
});

// Dispatch mesh tasks
cmd.cmdDrawMeshTasks({width, height, depth});
```

### Compute Shaders

```cpp
// Create compute pipeline
auto computePipeline = ctx->createComputePipeline({
    .smComp = computeShader,
    .debugName = "compute_pipeline"
});

// Dispatch compute
cmd.cmdBindComputePipeline(computePipeline);
cmd.cmdDispatchThreadGroups({width, height, depth});
```

## Memory Management

### Storage Types

```cpp
enum StorageType {
    StorageType_Device,        // GPU-only memory
    StorageType_HostVisible,  // CPU-accessible memory
    StorageType_Memoryless    // Transient memory (mobile GPUs)
};
```

### Buffer Operations

```cpp
// Upload data to buffer
ctx->upload(bufferHandle, data, size, offset);

// Download data from buffer
ctx->download(bufferHandle, data, size, offset);

// Get mapped pointer (for host-visible buffers)
uint8_t* ptr = ctx->getMappedPtr(bufferHandle);

// Get GPU address (for bindless rendering)
uint64_t gpuAddr = ctx->gpuAddress(bufferHandle);
```

## Vulkan Interop

LightweightVK provides seamless interop with raw Vulkan API:

```cpp
// Get underlying Vulkan objects
VkDevice vkDevice = lvk::getVkDevice(ctx.get());
VkCommandBuffer vkCmdBuf = lvk::getVkCommandBuffer(cmd);
VkBuffer vkBuffer = lvk::getVkBuffer(ctx.get(), bufferHandle);
VkImage vkImage = lvk::getVkImage(ctx.get(), textureHandle);

// Use raw Vulkan API
vkCmdUpdateBuffer(vkCmdBuf, vkBuffer, 0, sizeof(data), &data);
```

## Platform Support

### Supported Platforms
- **Windows**: Full Vulkan 1.3 support
- **Linux**: Full Vulkan 1.3 support  
- **macOS**: Vulkan 1.2 + extensions via MoltenVK
- **Android**: Vulkan 1.3 support

### Feature Support Matrix

| Feature | Windows | Linux | macOS | Android |
|---------|---------|-------|-------|---------|
| Vulkan 1.3 | ✅ | ✅ | ⚠️ | ✅ |
| Ray Tracing | ✅ | ✅ | ❌ | ✅ |
| Mesh Shaders | ✅ | ✅ | ❌ | ❌ |
| Dynamic Rendering | ✅ | ✅ | ✅ | ✅ |

## Best Practices

### 1. **Resource Management**
- Use `Holder<T>` for automatic resource cleanup
- Prefer move semantics over copying
- Use debug names for easier debugging

### 2. **Command Buffer Usage**
- Keep command buffer recording short
- Batch similar operations together
- Use debug labels for profiling

### 3. **Memory Optimization**
- Use appropriate storage types
- Prefer device memory for frequently accessed resources
- Use staging buffers for large uploads

### 4. **Performance**
- Minimize state changes
- Use push constants for small data
- Leverage bindless rendering for dynamic scenes

## Error Handling

```cpp
lvk::Result result;
auto buffer = ctx->createBuffer(desc, "buffer", &result);

if (!result.isOk()) {
    std::cerr << "Buffer creation failed: " << result.message << std::endl;
    return;
}
```

## Debugging

### Debug Labels
```cpp
cmd.cmdPushDebugGroupLabel("Render Scene", 0xff0000ff);
// ... rendering commands
cmd.cmdPopDebugGroupLabel();
```

### Validation Layers
```cpp
lvk::ContextConfig config{
    .enableValidation = true,
    .terminateOnValidationError = false
};
```

## Example: Complete Triangle

```cpp
#include <lvk/LVK.h>
#include <GLFW/glfw3.h>

const char* vertexShader = R"(
#version 460
layout(location=0) in vec3 position;
layout(location=0) out vec3 color;
void main() {
    gl_Position = vec4(position, 1.0);
    color = vec3(1.0, 0.0, 0.0);
}
)";

const char* fragmentShader = R"(
#version 460
layout(location=0) in vec3 color;
layout(location=0) out vec4 outColor;
void main() {
    outColor = vec4(color, 1.0);
}
)";

int main() {
    // Initialize
    glfwInit();
    GLFWwindow* window = lvk::initWindow("Triangle", 800, 600, false);
    auto ctx = lvk::createVulkanContextWithSwapchain(window, 800, 600, {});
    
    // Create resources
    float vertices[] = {
        0.0f, -0.5f, 0.0f,
        0.5f,  0.5f, 0.0f,
       -0.5f,  0.5f, 0.0f
    };
    
    auto vertexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Vertex,
        .storage = lvk::StorageType_HostVisible,
        .size = sizeof(vertices),
        .data = vertices
    });
    
    auto vertShader = ctx->createShaderModule({vertexShader, lvk::Stage_Vert, "vert"});
    auto fragShader = ctx->createShaderModule({fragmentShader, lvk::Stage_Frag, "frag"});
    
    auto pipeline = ctx->createRenderPipeline({
        .smVert = vertShader,
        .smFrag = fragShader,
        .color = {{.format = ctx->getSwapchainFormat()}}
    });
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        auto& cmd = ctx->acquireCommandBuffer();
        cmd.cmdBeginRendering(
            {.color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}}}},
            {.color = {{.texture = ctx->getCurrentSwapchainTexture()}}}
        );
        
        cmd.cmdBindRenderPipeline(pipeline);
        cmd.cmdBindVertexBuffer(0, vertexBuffer);
        cmd.cmdDraw(3);
        cmd.cmdEndRendering();
        
        ctx->submit(cmd, ctx->getCurrentSwapchainTexture());
    }
    
    return 0;
}
```

## cgltf Integration

### Overview

cgltf is a single-file C library for loading and parsing glTF 2.0 files. It's designed as a lightweight, header-only library that provides direct access to glTF data structures without heavy dependencies.

### Key Features

- **Single-file library**: Header-only implementation
- **glTF 2.0 support**: Full specification compliance
- **Binary and JSON**: Supports both .gltf and .glb formats
- **Extension support**: Many glTF extensions supported
- **Memory efficient**: Minimal overhead parsing
- **Cross-platform**: Pure C implementation

### Core Data Structures

#### cgltf_data
The main data structure containing all parsed glTF information:

```c
typedef struct cgltf_data {
    cgltf_file_type file_type;
    void* file_data;
    
    cgltf_asset asset;
    
    // Core glTF arrays
    cgltf_mesh* meshes;
    cgltf_size meshes_count;
    
    cgltf_material* materials;
    cgltf_size materials_count;
    
    cgltf_accessor* accessors;
    cgltf_size accessors_count;
    
    cgltf_buffer_view* buffer_views;
    cgltf_size buffer_views_count;
    
    cgltf_buffer* buffers;
    cgltf_size buffers_count;
    
    cgltf_image* images;
    cgltf_size images_count;
    
    cgltf_texture* textures;
    cgltf_size textures_count;
    
    cgltf_sampler* samplers;
    cgltf_size samplers_count;
    
    cgltf_node* nodes;
    cgltf_size nodes_count;
    
    cgltf_scene* scenes;
    cgltf_size scenes_count;
    
    cgltf_scene* scene;  // Default scene
    
    cgltf_animation* animations;
    cgltf_size animations_count;
    
    // Raw data
    const char* json;
    cgltf_size json_size;
    const void* bin;
    cgltf_size bin_size;
} cgltf_data;
```

#### Key Data Types

**cgltf_mesh**: Contains geometry data
```c
typedef struct cgltf_mesh {
    char* name;
    cgltf_primitive* primitives;
    cgltf_size primitives_count;
    cgltf_float* weights;
    cgltf_size weights_count;
    cgltf_morph_target* targets;
    cgltf_size targets_count;
} cgltf_mesh;
```

**cgltf_primitive**: Individual drawable geometry
```c
typedef struct cgltf_primitive {
    cgltf_primitive_type type;
    cgltf_accessor* indices;
    cgltf_material* material;
    cgltf_attribute* attributes;
    cgltf_size attributes_count;
} cgltf_primitive;
```

**cgltf_accessor**: Data accessor for vertex/index data
```c
typedef struct cgltf_accessor {
    char* name;
    cgltf_component_type component_type;
    cgltf_bool normalized;
    cgltf_type type;
    cgltf_size offset;
    cgltf_size count;
    cgltf_size stride;
    cgltf_buffer_view* buffer_view;
    cgltf_bool has_min;
    cgltf_float min[16];
    cgltf_bool has_max;
    cgltf_float max[16];
} cgltf_accessor;
```

### API Functions

#### Loading Functions

```c
// Parse glTF file from disk
cgltf_result cgltf_parse_file(
    const cgltf_options* options,
    const char* path,
    cgltf_data** out_data
);

// Parse glTF data from memory
cgltf_result cgltf_parse(
    const cgltf_options* options,
    const void* data,
    cgltf_size size,
    cgltf_data** out_data
);

// Load buffer data (external files)
cgltf_result cgltf_load_buffers(
    const cgltf_options* options,
    cgltf_data* data,
    const char* gltf_path
);

// Free allocated data
void cgltf_free(cgltf_data* data);
```

#### Data Access Functions

```c
// Unpack vertex data to float array
cgltf_size cgltf_accessor_unpack_floats(
    const cgltf_accessor* accessor,
    cgltf_float* out,
    cgltf_size float_count
);

// Read individual index
cgltf_size cgltf_accessor_read_index(
    const cgltf_accessor* accessor,
    cgltf_size index
);

// Read individual float value
cgltf_bool cgltf_accessor_read_float(
    const cgltf_accessor* accessor,
    cgltf_size index,
    cgltf_float* out,
    cgltf_size element_size
);

// Get buffer view data pointer
const uint8_t* cgltf_buffer_view_data(const cgltf_buffer_view* view);
```

#### Transform Functions

```c
// Get local transform matrix
void cgltf_node_transform_local(
    const cgltf_node* node,
    cgltf_float* out_matrix
);

// Get world transform matrix
void cgltf_node_transform_world(
    const cgltf_node* node,
    cgltf_float* out_matrix
);
```

### Integration with LightweightVK

#### Basic Loading Pattern

```cpp
#include <cgltf.h>

// Load glTF file
cgltf_data* LoadGLTF(const char* path) {
    cgltf_options options{};
    cgltf_data* data = nullptr;
    
    // Parse the file
    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success) {
        std::cerr << "Failed to parse glTF: " << path << std::endl;
        return nullptr;
    }
    
    // Load buffer data (external .bin files)
    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success) {
        std::cerr << "Failed to load buffers" << std::endl;
        cgltf_free(data);
        return nullptr;
    }
    
    return data;
}
```

#### Mesh Processing

```cpp
struct MeshBuffers {
    lvk::Holder<lvk::BufferHandle> vertexBuffer;
    lvk::Holder<lvk::BufferHandle> indexBuffer;
    uint32_t indexCount;
};

MeshBuffers UploadMesh(lvk::IContext* ctx, cgltf_primitive* prim) {
    MeshBuffers out{};
    
    // Find position and texture coordinate accessors
    cgltf_accessor* posAcc = nullptr;
    cgltf_accessor* texAcc = nullptr;
    cgltf_accessor* idxAcc = prim->indices;
    
    for (size_t i = 0; i < prim->attributes_count; ++i) {
        if (prim->attributes[i].type == cgltf_attribute_type_position) {
            posAcc = prim->attributes[i].data;
        } else if (prim->attributes[i].type == cgltf_attribute_type_texcoord) {
            texAcc = prim->attributes[i].data;
        }
    }
    
    if (!posAcc || !idxAcc) {
        throw std::runtime_error("Mesh missing required attributes");
    }
    
    size_t vertexCount = posAcc->count;
    size_t indexCount = idxAcc->count;
    out.indexCount = static_cast<uint32_t>(indexCount);
    
    // Extract vertex data
    std::vector<float> positions(vertexCount * 3);
    cgltf_accessor_unpack_floats(posAcc, positions.data(), positions.size());
    
    std::vector<float> texCoords(vertexCount * 2);
    if (texAcc) {
        cgltf_accessor_unpack_floats(texAcc, texCoords.data(), texCoords.size());
    } else {
        // Generate default UVs
        for (size_t i = 0; i < vertexCount; ++i) {
            texCoords[i * 2] = 0.0f;
            texCoords[i * 2 + 1] = 0.0f;
        }
    }
    
    // Create vertex buffer
    std::vector<Vertex> vertices(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i) {
        vertices[i].position = glm::vec3(
            positions[i * 3], 
            positions[i * 3 + 1], 
            positions[i * 3 + 2]
        );
        vertices[i].texCoord = glm::vec2(
            texCoords[i * 2], 
            texCoords[i * 2 + 1]
        );
    }
    
    // Create index buffer
    std::vector<uint32_t> indices(indexCount);
    for (size_t i = 0; i < indexCount; ++i) {
        indices[i] = static_cast<uint32_t>(cgltf_accessor_read_index(idxAcc, i));
    }
    
    // Upload to LightweightVK
    out.vertexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Vertex,
        .storage = lvk::StorageType_HostVisible,
        .size = vertices.size() * sizeof(Vertex),
        .data = vertices.data(),
        .debugName = "vertex_buffer"
    });
    
    out.indexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Index,
        .storage = lvk::StorageType_HostVisible,
        .size = indices.size() * sizeof(uint32_t),
        .data = indices.data(),
        .debugName = "index_buffer"
    });
    
    return out;
}
```

#### Complete Integration Example

```cpp
#include <cgltf.h>
#include <lvk/LVK.h>
#include <stb_image.h>

int main() {
    // Initialize LightweightVK
    auto ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    
    // Load glTF file
    cgltf_data* gltf = LoadGLTF("assets/model.gltf");
    if (!gltf) return -1;
    
    // Process all meshes
    std::vector<MeshBuffers> meshes;
    for (size_t mi = 0; mi < gltf->meshes_count; ++mi) {
        for (size_t pi = 0; pi < gltf->meshes[mi].primitives_count; ++pi) {
            try {
                MeshBuffers mesh = UploadMesh(ctx.get(), &gltf->meshes[mi].primitives[pi]);
                meshes.push_back(std::move(mesh));
            } catch (const std::exception& e) {
                std::cerr << "Failed to upload mesh: " << e.what() << std::endl;
            }
        }
    }
    
    // Load textures
    std::vector<lvk::Holder<lvk::TextureHandle>> textures;
    for (size_t i = 0; i < gltf->images_count; ++i) {
        if (gltf->images[i].uri) {
            auto texture = LoadTexture(ctx.get(), gltf->images[i].uri);
            if (texture.valid()) {
                textures.push_back(std::move(texture));
            }
        }
    }
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        auto& cmd = ctx->acquireCommandBuffer();
        cmd.cmdBeginRendering(/* ... */);
        
        // Render all meshes
        for (const auto& mesh : meshes) {
            cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
            cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
            cmd.cmdDrawIndexed(mesh.indexCount);
        }
        
        cmd.cmdEndRendering();
        ctx->submit(cmd, ctx->getCurrentSwapchainTexture());
    }
    
    // Cleanup
    cgltf_free(gltf);
    return 0;
}
```

### Advanced Features

#### Material Processing

```cpp
void ProcessMaterial(lvk::IContext* ctx, cgltf_material* material) {
    if (material->has_pbr_metallic_roughness) {
        auto& pbr = material->pbr_metallic_roughness;
        
        // Process base color texture
        if (pbr.base_color_texture.texture) {
            auto image = pbr.base_color_texture.texture->image;
            if (image && image->uri) {
                auto texture = LoadTexture(ctx, image->uri);
                // Bind texture to shader...
            }
        }
        
        // Process metallic-roughness texture
        if (pbr.metallic_roughness_texture.texture) {
            // Similar processing...
        }
    }
}
```

#### Animation Support

```cpp
void ProcessAnimations(cgltf_data* data) {
    for (size_t i = 0; i < data->animations_count; ++i) {
        auto& anim = data->animations[i];
        
        for (size_t j = 0; j < anim.channels_count; ++j) {
            auto& channel = anim.channels[j];
            
            // Process animation channel
            if (channel.target_node) {
                // Apply animation to node...
            }
        }
    }
}
```

#### Node Hierarchy

```cpp
void ProcessNode(cgltf_node* node, glm::mat4 parentTransform) {
    // Get local transform
    cgltf_float matrix[16];
    cgltf_node_transform_local(node, matrix);
    glm::mat4 localTransform = glm::make_mat4(matrix);
    
    glm::mat4 worldTransform = parentTransform * localTransform;
    
    // Process mesh if present
    if (node->mesh) {
        // Render mesh with worldTransform...
    }
    
    // Process children
    for (size_t i = 0; i < node->children_count; ++i) {
        ProcessNode(node->children[i], worldTransform);
    }
}
```

### Best Practices

1. **Memory Management**: Always call `cgltf_free()` on loaded data
2. **Error Handling**: Check return values from cgltf functions
3. **Buffer Loading**: Call `cgltf_load_buffers()` after parsing
4. **Data Validation**: Use `cgltf_validate()` for debugging
5. **Extension Support**: Check for required extensions before processing

### Supported Extensions

- **KHR_materials_pbrSpecularGlossiness**: Alternative PBR model
- **KHR_materials_unlit**: Unlit materials
- **KHR_texture_transform**: Texture coordinate transformations
- **KHR_materials_clearcoat**: Clearcoat materials
- **KHR_materials_transmission**: Transmissive materials
- **EXT_mesh_gpu_instancing**: GPU instancing
- **KHR_draco_mesh_compression**: Compressed geometry
- **KHR_texture_basisu**: Basis Universal textures

## Additional Dependencies

### GLM - OpenGL Mathematics Library

GLM is a header-only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications. It provides a comprehensive set of mathematical functions and types essential for 3D graphics programming.

#### Key Features
- **GLSL Compatibility**: Same naming conventions and functionality as GLSL
- **Header-Only**: No compilation required, just include headers
- **Cross-Platform**: Works on Windows, Linux, macOS, Android
- **Modern C++**: C++98 compatible with C++11/14/17/20 features
- **SIMD Optimized**: Automatic SIMD instruction usage when available
- **Comprehensive Math**: Vector, matrix, quaternion, and advanced mathematical operations
- **Performance**: Optimized for real-time graphics applications

#### Core Types
```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vector types
glm::vec2, glm::vec3, glm::vec4
glm::ivec2, glm::ivec3, glm::ivec4
glm::uvec2, glm::uvec3, glm::uvec4

// Matrix types
glm::mat2, glm::mat3, glm::mat4
glm::mat2x3, glm::mat3x4, etc.

// Quaternions
glm::quat
```

#### Common Operations
```cpp
// Vector operations
glm::vec3 v1(1.0f, 2.0f, 3.0f);
glm::vec3 v2(4.0f, 5.0f, 6.0f);
glm::vec3 result = v1 + v2;
float length = glm::length(v1);
glm::vec3 normalized = glm::normalize(v1);
float dot = glm::dot(v1, v2);
glm::vec3 cross = glm::cross(v1, v2);

// Advanced vector operations
glm::vec3 reflected = glm::reflect(incident, normal);
glm::vec3 refracted = glm::refract(incident, normal, eta);
float distance = glm::distance(v1, v2);
glm::vec3 lerped = glm::mix(v1, v2, 0.5f);

// Matrix operations
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(1.0f, 2.0f, 3.0f));
model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));

// Advanced matrix operations
glm::mat4 inverse = glm::inverse(model);
glm::mat4 transpose = glm::transpose(model);
float determinant = glm::determinant(model);

// Projection matrices
glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f/9.0f, 0.1f, 100.0f);
glm::mat4 ortho = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
glm::mat4 frustum = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

// View matrix
glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));

// Quaternion operations
glm::quat rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
glm::quat slerped = glm::slerp(q1, q2, 0.5f);
```

#### Advanced Features
```cpp
// Noise functions
float noise = glm::simplex(glm::vec2(x, y));
float perlin = glm::perlin(glm::vec3(x, y, z));

// Color space conversions
glm::vec3 rgb = glm::vec3(1.0f, 0.5f, 0.0f);
glm::vec3 hsv = glm::rgb2hsv(rgb);
glm::vec3 backToRgb = glm::hsv2rgb(hsv);

// Random number generation
glm::vec3 randomVec = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
float randomFloat = glm::linearRand(0.0f, 1.0f);

// Mathematical constants
const float pi = glm::pi<float>();
const float e = glm::e<float>();
const float sqrt2 = glm::root_two<float>();
```

#### Integration with LightweightVK
```cpp
// MVP matrix setup
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 proj = camera.GetProjectionMatrix();
glm::mat4 mvp = proj * view * model;

// Push constants
struct PushConstants {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
PushConstants pc{model, view, proj};
cmd.cmdPushConstants(pc, lvk::Stage_Vert);
```

### GLFW - Window Management Library

GLFW is a multi-platform library for OpenGL, OpenGL ES and Vulkan application development.

#### Key Features
- **Cross-Platform**: Windows, macOS, Linux support
- **Vulkan Support**: Native Vulkan surface creation
- **Input Handling**: Keyboard, mouse, joystick support
- **Window Management**: Create, resize, fullscreen windows
- **Context Creation**: OpenGL/Vulkan context management

#### Basic Usage
```cpp
#include <GLFW/glfw3.h>

// Initialize GLFW
glfwInit();

// Create window
GLFWwindow* window = glfwCreateWindow(800, 600, "My App", nullptr, nullptr);
glfwMakeContextCurrent(window);

// Main loop
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    
    // Rendering code here
    
    glfwSwapBuffers(window);
}

// Cleanup
glfwDestroyWindow(window);
glfwTerminate();
```

#### Input Handling
```cpp
// Keyboard input
if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    // Move forward
}

// Mouse input
glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
    // Handle mouse movement
});

// Mouse button input
glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Left mouse button pressed
    }
});
```

#### Integration with LightweightVK
```cpp
// Create window for LightweightVK
GLFWwindow* window = lvk::initWindow("Vulkan App", 800, 600, false);

// Create Vulkan context
auto ctx = lvk::createVulkanContextWithSwapchain(window, 800, 600, {});
```

### spdlog - Fast C++ Logging Library

spdlog is a very fast, header-only/compiled, C++ logging library.

#### Key Features
- **High Performance**: Very fast logging with minimal overhead
- **Header-Only**: Can be used as header-only library
- **Multiple Sinks**: Console, files, syslog, Windows event log
- **Thread-Safe**: Multi-threaded logging support
- **Rich Formatting**: Uses fmt library for formatting

#### Basic Usage
```cpp
#include "spdlog/spdlog.h"

// Basic logging
spdlog::info("Welcome to spdlog!");
spdlog::error("Some error message with arg: {}", 1);
spdlog::warn("Easy padding in numbers like {:08d}", 12);
spdlog::critical("Support for int: {0:d}; hex: {0:x}; oct: {0:o}; bin: {0:b}", 42);

// Set log level
spdlog::set_level(spdlog::level::debug);
spdlog::debug("This message should be displayed..");

// Custom pattern
spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
```

### STB - Single-File Public Domain Libraries

STB is a collection of single-file public domain libraries for C/C++.

#### Key Libraries Used

**stb_image.h** - Image Loading
```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Load image
int width, height, channels;
unsigned char* data = stbi_load("texture.png", &width, &height, &channels, 0);
if (data) {
    // Use image data
    stbi_image_free(data);
}
```

#### Integration with LightweightVK
```cpp
// Load texture for LightweightVK
lvk::Holder<lvk::TextureHandle> LoadTexture(lvk::IContext* ctx, const char* path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return {};
    }
    
    auto texture = ctx->createTexture({
        .type = lvk::TextureType_2D,
        .format = lvk::Format_RGBA_UN8,
        .dimensions = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = lvk::TextureUsageBits_Sampled,
        .data = data,
        .debugName = "texture"
    });
    
    stbi_image_free(data);
    return texture;
}
```

### yaml-cpp - YAML Parser and Emitter

yaml-cpp is a YAML parser and emitter in C++ matching the YAML 1.2 spec.

#### Key Features
- **YAML 1.2 Compliance**: Full specification support
- **Easy Integration**: Simple C++ API
- **Cross-Platform**: Works on all major platforms
- **CMake Support**: Easy integration with CMake projects

#### Basic Usage
```cpp
#include <yaml-cpp/yaml.h>

// Parse YAML file
YAML::Node config = YAML::LoadFile("config.yaml");

// Access values
std::string name = config["name"].as<std::string>();
int width = config["window"]["width"].as<int>();
int height = config["window"]["height"].as<int>();

// Iterate over sequences
for (const auto& item : config["items"]) {
    std::string value = item.as<std::string>();
}
```

### minilog - Minimalistic Logging Library

minilog is a minimalistic logging library with threads and manual callstacks.

#### Key Features
- **Easy to Use**: 2 files, C-style interface
- **Multiple Outputs**: Text log, HTML log, Android logcat, console
- **Thread-Safe**: Write from multiple threads
- **Cross-Platform**: Windows, Linux, macOS, Android
- **Callbacks**: Intercept formatted log messages

#### Basic Usage
```cpp
#include "minilog/minilog.h"

// Initialize logging
minilog::initialize("log.txt", {});

// Log messages
minilog::log(minilog::Log, "Hello world!");
minilog::log(minilog::Warning, "Warning!!!");
minilog::log(minilog::Error, "Error occurred: %s", errorMessage);

// Cleanup
minilog::deinitialize();
```

## Advanced Vulkan Features with LightweightVK

### Vulkan 1.3 Dynamic Rendering

LightweightVK leverages Vulkan 1.3's dynamic rendering feature, eliminating the need for traditional render passes and framebuffers.

#### Dynamic Rendering Example
```cpp
// Begin dynamic rendering
lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
cmd.cmdBeginRendering({
    .color = {{
        .loadOp = lvk::LoadOp_Clear,
        .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}
    }},
    .depth = {
        .loadOp = lvk::LoadOp_Clear,
        .clearDepth = 1.0f
    }
}, {
    .color = {{.texture = ctx->getCurrentSwapchainTexture()}},
    .depth = {.texture = depthTexture}
});

// Bind pipeline and render
cmd.cmdBindRenderPipeline(pipeline);
cmd.cmdDraw(3, 1, 0, 0);

// End dynamic rendering
cmd.cmdEndRendering();
```

### Bindless Rendering with Descriptor Indexing

LightweightVK's bindless architecture allows for efficient resource management without traditional descriptor sets.

#### Bindless Texture Access
```cpp
// Create bindless texture array
lvk::TextureDesc textureArrayDesc{
    .type = lvk::TextureType_2D,
    .format = lvk::Format_RGBA_UN8,
    .dimensions = {1024, 1024, 1},
    .arrayLayers = 1000,  // Support up to 1000 textures
    .usage = lvk::TextureUsageBits_Sampled,
    .debugName = "bindless_texture_array"
};
auto bindlessTextureArray = ctx->createTexture(textureArrayDesc);

// In shader, access textures by index
// layout(set = 0, binding = 0) uniform texture2D textures[];
// layout(set = 0, binding = 1) uniform sampler samplers[];
```

### Ray Tracing Integration

LightweightVK supports Vulkan ray tracing extensions for modern rendering techniques.

#### Ray Tracing Setup
```cpp
// Create acceleration structure
lvk::AccelStructDesc accelDesc{
    .type = lvk::AccelStructType_BottomLevel,
    .flags = lvk::AccelStructBuildFlags_PreferFastTrace,
    .geometryCount = 1,
    .geometries = {{
        .type = lvk::AccelStructGeometryType_Triangles,
        .triangles = {
            .vertexData = vertexBuffer,
            .indexData = indexBuffer,
            .maxVertex = vertexCount - 1,
            .vertexStride = sizeof(Vertex)
        }
    }},
    .debugName = "blas"
};
auto blas = ctx->createAccelStruct(accelDesc);

// Create top-level acceleration structure
lvk::AccelStructDesc tlasDesc{
    .type = lvk::AccelStructType_TopLevel,
    .flags = lvk::AccelStructBuildFlags_PreferFastTrace,
    .instanceCount = 1,
    .instances = {{
        .accelStruct = blas,
        .transform = glm::mat3x4(1.0f),
        .instanceId = 0,
        .mask = 0xFF
    }},
    .debugName = "tlas"
};
auto tlas = ctx->createAccelStruct(tlasDesc);
```

### Mesh Shaders Support

LightweightVK supports the VK_EXT_mesh_shader extension for modern GPU-driven rendering.

#### Mesh Shader Pipeline
```cpp
// Create mesh shader pipeline
lvk::RenderPipelineDesc meshPipelineDesc{
    .smMesh = meshShader,
    .smTask = taskShader,
    .smFrag = fragmentShader,
    .color = {{.format = ctx->getSwapchainFormat()}},
    .debugName = "mesh_shader_pipeline"
};
auto meshPipeline = ctx->createRenderPipeline(meshPipelineDesc);

// Dispatch mesh shader
cmd.cmdBindRenderPipeline(meshPipeline);
cmd.cmdDrawMeshTasks(1, 1, 1);  // Dispatch 1x1x1 workgroups
```

## Complete Integration Example

Here's a comprehensive example showing how all libraries work together in a modern Vulkan application:

```cpp
#include <GLFW/glfw3.h>
#include <lvk/LVK.h>
#include <cgltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <stb_image.h>

class VulkanEngine {
private:
    std::unique_ptr<lvk::IContext> ctx_;
    GLFWwindow* window_;
    std::vector<MeshBuffers> meshes_;
    std::vector<lvk::Holder<lvk::TextureHandle>> textures_;
    lvk::Holder<lvk::RenderPipelineHandle> pipeline_;
    lvk::Holder<lvk::SamplerHandle> sampler_;
    
    // Camera
    glm::vec3 cameraPos_{0.0f, 0.0f, 5.0f};
    glm::vec3 cameraTarget_{0.0f, 0.0f, 0.0f};
    glm::mat4 viewMatrix_;
    glm::mat4 projMatrix_;

public:
    bool Initialize() {
        // Initialize logging
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("Initializing Vulkan Engine");
        
        // Initialize GLFW
        if (!glfwInit()) {
            spdlog::error("Failed to initialize GLFW");
            return false;
        }
        
        // Create window
        window_ = lvk::initWindow("Vulkan Engine", 800, 600, false);
        if (!window_) {
            spdlog::error("Failed to create window");
            return false;
        }
        
        // Create Vulkan context
        ctx_ = lvk::createVulkanContextWithSwapchain(window_, 800, 600, {});
        if (!ctx_) {
            spdlog::error("Failed to create Vulkan context");
            return false;
        }
        
        // Load scene configuration
        if (!LoadSceneConfig()) {
            return false;
        }
        
        // Load 3D models
        if (!LoadModels()) {
            return false;
        }
        
        // Create rendering pipeline
        if (!CreatePipeline()) {
            return false;
        }
        
        spdlog::info("Vulkan Engine initialized successfully");
        return true;
    }
    
    bool LoadSceneConfig() {
        try {
            YAML::Node config = YAML::LoadFile("assets/scenes/default.yml");
            
            // Process camera settings
            if (config["camera"]) {
                auto pos = config["camera"]["position"];
                cameraPos_ = glm::vec3(
                    pos["x"].as<float>(),
                    pos["y"].as<float>(),
                    pos["z"].as<float>()
                );
            }
            
            spdlog::info("Scene configuration loaded");
            return true;
        } catch (const std::exception& e) {
            spdlog::error("Failed to load scene config: {}", e.what());
            return false;
        }
    }
    
    bool LoadModels() {
        cgltf_data* gltf = LoadGLTF("assets/models/car.gltf");
        if (!gltf) {
            spdlog::error("Failed to load glTF model");
            return false;
        }
        
        // Process meshes
        for (size_t mi = 0; mi < gltf->meshes_count; ++mi) {
            for (size_t pi = 0; pi < gltf->meshes[mi].primitives_count; ++pi) {
                try {
                    MeshBuffers mesh = UploadMesh(ctx_.get(), &gltf->meshes[mi].primitives[pi]);
                    meshes_.push_back(std::move(mesh));
                    spdlog::debug("Uploaded mesh: {} indices", mesh.indexCount);
                } catch (const std::exception& e) {
                    spdlog::error("Failed to upload mesh: {}", e.what());
                }
            }
        }
        
        // Load textures
        for (size_t i = 0; i < gltf->images_count; ++i) {
            if (gltf->images[i].uri) {
                std::string texturePath = "assets/models/" + std::string(gltf->images[i].uri);
                auto texture = LoadTexture(ctx_.get(), texturePath.c_str());
                if (texture.valid()) {
                    textures_.push_back(std::move(texture));
                }
            }
        }
        
        cgltf_free(gltf);
        spdlog::info("Loaded {} meshes and {} textures", meshes_.size(), textures_.size());
        return true;
    }
    
    bool CreatePipeline() {
        // Load shaders
        auto vertShader = ctx_->createShaderModule({
            ReadFile("shaders/basic.vert").c_str(),
            lvk::Stage_Vert,
            "vertex_shader"
        });
        
        auto fragShader = ctx_->createShaderModule({
            ReadFile("shaders/basic.frag").c_str(),
            lvk::Stage_Frag,
            "fragment_shader"
        });
        
        // Create pipeline
        lvk::RenderPipelineDesc pipelineDesc{
            .smVert = vertShader,
            .smFrag = fragShader,
            .color = {{.format = ctx_->getSwapchainFormat()}},
            .vertexInput = {
                .attributes = {
                    {.location = 0, .binding = 0, .format = lvk::VertexFormat::Float3, .offset = 0},
                    {.location = 1, .binding = 0, .format = lvk::VertexFormat::Float2, .offset = 12}
                },
                .inputBindings = {{.stride = 20}}
            },
            .debugName = "main_pipeline"
        };
        
        pipeline_ = ctx_->createRenderPipeline(pipelineDesc);
        
        // Create sampler
        sampler_ = ctx_->createSampler({
            .minFilter = lvk::SamplerFilter_Linear,
            .magFilter = lvk::SamplerFilter_Linear,
            .mipMap = lvk::SamplerMip_Linear,
            .wrapU = lvk::SamplerWrap_Repeat,
            .wrapV = lvk::SamplerWrap_Repeat,
            .debugName = "texture_sampler"
        });
        
        return pipeline_.valid();
    }
    
    void UpdateCamera() {
        // Handle input
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos_ += glm::normalize(cameraTarget_ - cameraPos_) * 0.1f;
            cameraTarget_ += glm::normalize(cameraTarget_ - cameraPos_) * 0.1f;
        }
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos_ -= glm::normalize(cameraTarget_ - cameraPos_) * 0.1f;
            cameraTarget_ -= glm::normalize(cameraTarget_ - cameraPos_) * 0.1f;
        }
        
        // Update matrices
        viewMatrix_ = glm::lookAt(cameraPos_, cameraTarget_, glm::vec3(0.0f, 1.0f, 0.0f));
        projMatrix_ = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
    }
    
    void Render() {
        UpdateCamera();
        
        // Push constants
        struct PushConstants {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };
        
        PushConstants pc{
            .model = glm::mat4(1.0f),
            .view = viewMatrix_,
            .proj = projMatrix_
        };
        
        // Begin rendering
        auto& cmd = ctx_->acquireCommandBuffer();
        cmd.cmdBeginRendering(
            {.color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}}}},
            {.color = {{.texture = ctx_->getCurrentSwapchainTexture()}}}
        );
        
        cmd.cmdBindRenderPipeline(pipeline_);
        cmd.cmdPushConstants(pc, lvk::Stage_Vert);
        
        // Render all meshes
        for (const auto& mesh : meshes_) {
            cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
            cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
            cmd.cmdDrawIndexed(mesh.indexCount);
        }
        
        cmd.cmdEndRendering();
        ctx_->submit(cmd, ctx_->getCurrentSwapchainTexture());
    }
    
    void Run() {
        while (!glfwWindowShouldClose(window_)) {
            glfwPollEvents();
            Render();
        }
    }
    
    void Shutdown() {
        meshes_.clear();
        textures_.clear();
        pipeline_.release();
        sampler_.release();
        ctx_.release();
        glfwDestroyWindow(window_);
        glfwTerminate();
        spdlog::info("Vulkan Engine shutdown complete");
    }
};

int main() {
    VulkanEngine engine;
    
    if (!engine.Initialize()) {
        spdlog::error("Failed to initialize engine");
        return -1;
    }
    
    engine.Run();
    engine.Shutdown();
    
    return 0;
}
```

## Performance Optimization Tips

### Memory Management
- Use VMA for efficient memory allocation
- Implement texture streaming for large assets
- Use bindless rendering to reduce descriptor set overhead

### Rendering Optimization
- Leverage dynamic rendering for reduced API overhead
- Use mesh shaders for GPU-driven rendering
- Implement frustum culling and occlusion culling

### Asset Loading
- Use glTF for standardized 3D assets
- Implement asynchronous asset loading
- Use texture compression formats (BC, ASTC, ETC2)

This documentation provides a comprehensive overview of LightweightVK's architecture, API, and usage patterns, including full integration with cgltf for 3D asset loading and all supporting libraries. The library's design emphasizes modern Vulkan features while maintaining simplicity and performance.
