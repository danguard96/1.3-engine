//
// Created by Yibuz Pokopodrozo on 2025-09-16.
//

#include <filesystem>
#include <iostream>
#include <GLFW/glfw3.h>
#include <lightweightvk/lvk/LVK.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cgltf.h>
#include "Camera.h"
#include <stb_image.h>

std::string ReadFile(const std::filesystem::path &shader_path) {
    if (!exists(shader_path) || !is_regular_file(shader_path)) return {};

    std::ifstream file(shader_path);
    if (!file.is_open()) return {};

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper: read entire glTF file
cgltf_data* LoadGLTF(const char* path) {
    cgltf_options options{};
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success) {
        std::cerr << "Failed to parse glTF: " << path << "\n";
        return nullptr;
    }
    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success) {
        std::cerr << "Failed to load buffers\n";
        cgltf_free(data);
        return nullptr;
    }
    return data;
}

// Helper: Load texture from file
lvk::Holder<lvk::TextureHandle> LoadTexture(lvk::IContext* ctx, const char* path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4); // Force RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return {};
    }
    
    // Create texture with data directly (lvk style)
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

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

struct MeshBuffers {
    lvk::Holder<lvk::BufferHandle> vertexBuffer;
    lvk::Holder<lvk::BufferHandle> indexBuffer;
    uint32_t indexCount;
    
    // Make it movable but not copyable
    MeshBuffers() = default;
    MeshBuffers(const MeshBuffers&) = delete;
    MeshBuffers& operator=(const MeshBuffers&) = delete;
    MeshBuffers(MeshBuffers&&) = default;
    MeshBuffers& operator=(MeshBuffers&&) = default;
};

MeshBuffers UploadMesh(lvk::IContext* ctx, cgltf_primitive* prim) {
    MeshBuffers out{};

    // --- Gather positions and texture coordinates
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
        throw std::runtime_error("Mesh has no positions or indices");
    }

    size_t vertexCount = posAcc->count;
    size_t indexCount  = idxAcc->count;
    out.indexCount     = static_cast<uint32_t>(indexCount);

    // Extract positions
    std::vector<float> positions(vertexCount * 3);
    cgltf_accessor_unpack_floats(posAcc, positions.data(), positions.size());

    // Extract texture coordinates (use default if not available)
    std::vector<float> texCoords(vertexCount * 2);
    if (texAcc) {
        cgltf_accessor_unpack_floats(texAcc, texCoords.data(), texCoords.size());
    } else {
        // Default texture coordinates if not available
        for (size_t i = 0; i < vertexCount; ++i) {
            texCoords[i * 2] = 0.0f;
            texCoords[i * 2 + 1] = 0.0f;
        }
    }

    // Create vertex data with position and texture coordinates
    std::vector<Vertex> vertices(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i) {
        vertices[i].position = glm::vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
        vertices[i].texCoord = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);
    }

    std::vector<uint32_t> indices(indexCount);
    for (size_t i = 0; i < indexCount; ++i) {
        indices[i] = static_cast<uint32_t>(cgltf_accessor_read_index(idxAcc, i));
    }
    
    // Debug: Print first few vertices to see what we're actually getting
    std::cout << "Mesh vertices: ";
    for (int i = 0; i < 3 && i < vertexCount; ++i) {
        std::cout << "(" << positions[i*3] << ", " << positions[i*3+1] << ", " << positions[i*3+2] << ") ";
    }
    std::cout << std::endl;
    

    // --- Upload via LightweightVK
    {
        lvk::BufferDesc vdesc{};
        vdesc.size = vertices.size() * sizeof(Vertex);
        vdesc.usage = lvk::BufferUsageBits_Vertex;
        vdesc.storage = lvk::StorageType_HostVisible;

        out.vertexBuffer = ctx->createBuffer(vdesc, "vertex_buffer");
        
        // Upload data
        Vertex* vtx = (Vertex*)ctx->getMappedPtr(out.vertexBuffer);
        memcpy(vtx, vertices.data(), vertices.size() * sizeof(Vertex));
    }
    {
        lvk::BufferDesc idesc{};
        idesc.size = indices.size() * sizeof(uint32_t);
        idesc.usage = lvk::BufferUsageBits_Index;
        idesc.storage = lvk::StorageType_HostVisible;

        out.indexBuffer = ctx->createBuffer(idesc, "index_buffer");
        
        // Upload data
        uint32_t* idx = (uint32_t*)ctx->getMappedPtr(out.indexBuffer);
        memcpy(idx, indices.data(), indices.size() * sizeof(uint32_t));
    }

    return out;
}

int main(int argc, char *argv[]) {
    glfwInit();
    int width{800};
    int height{600};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    
    // Camera setup
    Camera camera;
    glm::vec3 camPos(0.0f, 0.0f, 5.0f);
    glm::vec3 camTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 camUp(0.0f, 1.0f, 0.0f);
    
    // Set up perspective projection
    camera.Perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    camera.LookAt(camPos, camTarget, camUp);
    
    // Load glTF asset
    cgltf_data* gltf = LoadGLTF("assets/old_rusty_car/scene.gltf");
    if (!gltf) return -1;
    
    // Load all textures from the glTF file
    auto texture0 = LoadTexture(ctx.get(), "assets/old_rusty_car/textures/Material_294_baseColor.png");        // Material_294 baseColor
    auto texture1 = LoadTexture(ctx.get(), "assets/old_rusty_car/textures/Material_294_metallicRoughness.png"); // Material_294 metallicRoughness  
    auto texture2 = LoadTexture(ctx.get(), "assets/old_rusty_car/textures/Material_294_normal.png");          // Material_294 normal
    auto texture3 = LoadTexture(ctx.get(), "assets/old_rusty_car/textures/Material_295_baseColor.png");        // Material_295 baseColor
    auto texture4 = LoadTexture(ctx.get(), "assets/old_rusty_car/textures/Material_316_baseColor.png");        // Material_316 baseColor
    
    std::cout << "Loaded all textures successfully" << std::endl;

    std::vector<MeshBuffers> meshes;
    for (size_t mi = 0; mi < gltf->meshes_count; ++mi) {
        for (size_t pi = 0; pi < gltf->meshes[mi].primitives_count; ++pi) {
            try {
                MeshBuffers mesh = UploadMesh(ctx.get(), &gltf->meshes[mi].primitives[pi]);
                meshes.push_back(std::move(mesh));
                std::cout << "Uploaded mesh: " << meshes.back().indexCount << " indices\n";
            } catch (const std::exception& e) {
                std::cerr << "Failed to upload mesh: " << e.what() << std::endl;
            }
        }
    }

    // Create shaders
    const lvk::Holder<lvk::ShaderModuleHandle> vert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/basic.vert").c_str()), lvk::Stage_Vert, ("vert shader")}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> frag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/basic.frag").c_str()), lvk::Stage_Frag, ("frag shader")}, nullptr);
    
    // Create pipeline
    lvk::RenderPipelineDesc pipelineDesc{};
    pipelineDesc.smVert = vert;
    pipelineDesc.smFrag = frag;
    pipelineDesc.color[0].format = ctx->getSwapchainFormat();
    
    // Set up vertex input (position + texture coordinates)
    pipelineDesc.vertexInput.attributes[0] = {.location = 0, .binding = 0, .format = lvk::VertexFormat::Float3, .offset = 0};
    pipelineDesc.vertexInput.attributes[1] = {.location = 1, .binding = 0, .format = lvk::VertexFormat::Float2, .offset = 12};
    pipelineDesc.vertexInput.inputBindings[0] = {.stride = 20}; // 3 floats + 2 floats * 4 bytes = 20 bytes
    
    const lvk::Holder<lvk::RenderPipelineHandle> pipeline = ctx->createRenderPipeline(pipelineDesc);
    
    // Create sampler for textures
    auto sampler = ctx->createSampler({
        .minFilter = lvk::SamplerFilter_Linear,
        .magFilter = lvk::SamplerFilter_Linear,
        .mipMap = lvk::SamplerMip_Linear,
        .wrapU = lvk::SamplerWrap_Repeat,
        .wrapV = lvk::SamplerWrap_Repeat,
        .debugName = "texture_sampler"
    }, nullptr);
    
    // Push constants for MVP matrices only
    struct PushConstants {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwGetFramebufferSize(window, &width, &height);
        if (!width || !height) {
            continue;
        }
        
        // Camera movement
        float moveSpeed = 0.003f; // Even slower movement
        float lookSpeed = 0.005f; // Keep rotation speed the same
        glm::vec3 forward = glm::normalize(camTarget - camPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, camUp));
        
        // WASD movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camPos += forward * moveSpeed;
            camTarget += forward * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camPos -= forward * moveSpeed;
            camTarget -= forward * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camPos -= right * moveSpeed;
            camTarget -= right * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camPos += right * moveSpeed;
            camTarget += right * moveSpeed;
        }
        
        // Arrow keys for looking around
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            camTarget -= camUp * lookSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            camTarget += camUp * lookSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            camTarget -= right * lookSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            camTarget += right * lookSpeed;
        }
        
        // Update camera
        camera.LookAt(camPos, camTarget, camUp);
        
        // Update push constants with MVP matrices only
        PushConstants pc{};
        pc.model = glm::mat4(1.0f); // Identity matrix for now
        pc.view = camera.GetViewMatrix();
        pc.proj = camera.GetProjectionMatrix();
        
        lvk::ICommandBuffer &command_buffer = ctx->acquireCommandBuffer();
        command_buffer.cmdBeginRendering({.color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}}}},
                                         {.color = {{.texture = ctx->getCurrentSwapchainTexture()}}});
        command_buffer.cmdBindRenderPipeline(pipeline);
        command_buffer.cmdPushConstants(pc, lvk::Stage_Vert);
        
        // Test: Render a simple hardcoded triangle first
        if (false) { // Set to true to test with hardcoded triangle
            // Create a simple test triangle
            float testVertices[] = {
                -0.5f, -0.5f, 0.0f,  // Bottom left
                 0.5f, -0.5f, 0.0f,  // Bottom right  
                 0.0f,  0.5f, 0.0f   // Top
            };
            uint32_t testIndices[] = {0, 1, 2};
            
            // Create test buffers
            lvk::BufferDesc vdesc{};
            vdesc.size = sizeof(testVertices);
            vdesc.usage = lvk::BufferUsageBits_Vertex;
            vdesc.storage = lvk::StorageType_HostVisible;
            auto testVertexBuffer = ctx->createBuffer(vdesc, "test_vertex");
            
            lvk::BufferDesc idesc{};
            idesc.size = sizeof(testIndices);
            idesc.usage = lvk::BufferUsageBits_Index;
            idesc.storage = lvk::StorageType_HostVisible;
            auto testIndexBuffer = ctx->createBuffer(idesc, "test_index");
            
            // Upload test data
            float* vtx = (float*)ctx->getMappedPtr(testVertexBuffer);
            memcpy(vtx, testVertices, sizeof(testVertices));
            
            uint32_t* idx = (uint32_t*)ctx->getMappedPtr(testIndexBuffer);
            memcpy(idx, testIndices, sizeof(testIndices));
            
            // Render test triangle
            command_buffer.cmdBindVertexBuffer(0, testVertexBuffer);
            command_buffer.cmdBindIndexBuffer(testIndexBuffer, lvk::IndexFormat_UI32);
            command_buffer.cmdDrawIndexed(3);
        } else {
            // Render only the first mesh to debug
            if (!meshes.empty()) {
                command_buffer.cmdBindVertexBuffer(0, meshes[0].vertexBuffer);
                command_buffer.cmdBindIndexBuffer(meshes[0].indexBuffer, lvk::IndexFormat_UI32);
                command_buffer.cmdDrawIndexed(meshes[0].indexCount);
            }
        }

        command_buffer.cmdEndRendering();
        ctx->submit(command_buffer, ctx->getCurrentSwapchainTexture());
    }
    
    cgltf_free(gltf);
    ctx.release();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}