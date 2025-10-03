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

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
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

// Helper: Load FBX model using assimp
const aiScene* LoadFBX(const char* path) {
    const aiScene* scene = aiImportFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    
    if (!scene) {
        std::cerr << "Failed to load FBX: " << path << "\n";
        return nullptr;
    }
    
    if (!scene->HasMeshes()) {
        std::cerr << "No meshes found in FBX file: " << path << "\n";
        aiReleaseImport(scene);
        return nullptr;
    }
    
    return scene;
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
    glm::vec3 normal;
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

MeshBuffers UploadMesh(lvk::IContext* ctx, const aiMesh* mesh) {
    MeshBuffers out{};

    if (!mesh->HasPositions()) {
        throw std::runtime_error("Mesh has no positions");
    }

    size_t vertexCount = mesh->mNumVertices;
    size_t indexCount = mesh->mNumFaces * 3; // Triangulated
    out.indexCount = static_cast<uint32_t>(indexCount);

    // Create vertex data with position, texture coordinates, and normals
    std::vector<Vertex> vertices(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i) {
        vertices[i].position = glm::vec3(
            mesh->mVertices[i].x, 
            mesh->mVertices[i].y, 
            mesh->mVertices[i].z
        );
        
        // Texture coordinates (use first UV channel if available)
        if (mesh->HasTextureCoords(0)) {
            vertices[i].texCoord = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        } else {
            vertices[i].texCoord = glm::vec2(0.0f, 0.0f);
        }
        
        // Normals
        if (mesh->HasNormals()) {
            vertices[i].normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        } else {
            vertices[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    // Extract indices
    std::vector<uint32_t> indices(indexCount);
    size_t idx = 0;
    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j) {
            indices[idx++] = face.mIndices[j];
        }
    }
    
    // Debug: Print first few vertices
    std::cout << "Mesh vertices: ";
    for (int i = 0; i < 3 && i < vertexCount; ++i) {
        std::cout << "(" << vertices[i].position.x << ", " << vertices[i].position.y << ", " << vertices[i].position.z << ") ";
    }
    std::cout << std::endl;

    // --- Upload via LightweightVK
    {
        lvk::BufferDesc vdesc{};
        vdesc.size = vertices.size() * sizeof(Vertex);
        vdesc.usage = lvk::BufferUsageBits_Vertex;
        vdesc.storage = lvk::StorageType_Device;

        out.vertexBuffer = ctx->createBuffer(vdesc, "vertex_buffer");
        
        // Upload data
        Vertex* vtx = (Vertex*)ctx->getMappedPtr(out.vertexBuffer);
        memcpy(vtx, vertices.data(), vertices.size() * sizeof(Vertex));
    }
    {
        lvk::BufferDesc idesc{};
        idesc.size = indices.size() * sizeof(uint32_t);
        idesc.usage = lvk::BufferUsageBits_Index;
        idesc.storage = lvk::StorageType_Device;

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
    
    // Load FBX asset
    const aiScene* scene = LoadFBX("assets/skull/source/skull.fbx");
    if (!scene) return -1;
    
    // Load textures for the skull model
    auto texture0 = LoadTexture(ctx.get(), "assets/skull/textures/skullColor.png");
    auto texture1 = LoadTexture(ctx.get(), "assets/skull/textures/skullNormal.png");
    auto texture2 = LoadTexture(ctx.get(), "assets/skull/textures/skullRoughness.png");
    
    std::cout << "Loaded all textures successfully" << std::endl;

    std::vector<MeshBuffers> meshes;
    for (size_t mi = 0; mi < scene->mNumMeshes; ++mi) {
        try {
            MeshBuffers mesh = UploadMesh(ctx.get(), scene->mMeshes[mi]);
            meshes.push_back(std::move(mesh));
            std::cout << "Uploaded mesh " << mi << ": " << meshes.back().indexCount << " indices\n";
        } catch (const std::exception& e) {
            std::cerr << "Failed to upload mesh " << mi << ": " << e.what() << std::endl;
        }
    }

    // Create shaders
    const lvk::Holder<lvk::ShaderModuleHandle> vert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/blinn_phong.vert").c_str()), lvk::Stage_Vert, ("vert shader")}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> frag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/blinn_phong.frag").c_str()), lvk::Stage_Frag, ("frag shader")}, nullptr);
    
    // Create pipeline
    lvk::RenderPipelineDesc pipelineDesc{};
    pipelineDesc.smVert = vert;
    pipelineDesc.smFrag = frag;
    pipelineDesc.color[0].format = ctx->getSwapchainFormat();
    
    // Set up vertex input (position + texture coordinates + normals)
    pipelineDesc.vertexInput.attributes[0] = {.location = 0, .binding = 0, .format = lvk::VertexFormat::Float3, .offset = 0};
    pipelineDesc.vertexInput.attributes[1] = {.location = 1, .binding = 0, .format = lvk::VertexFormat::Float2, .offset = 12};
    pipelineDesc.vertexInput.attributes[2] = {.location = 2, .binding = 0, .format = lvk::VertexFormat::Float3, .offset = 20};
    pipelineDesc.vertexInput.inputBindings[0] = {.stride = 32}; // 3 + 2 + 3 floats * 4 bytes = 32 bytes
    
    const lvk::Holder<lvk::RenderPipelineHandle> pipeline = ctx->createRenderPipeline(pipelineDesc);

    // Create depth texture
    auto depthTexture = ctx->createTexture({
        .type = lvk::TextureType_2D,
        .format = lvk::Format_Z_F32,
        .dimensions = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = lvk::TextureUsageBits_Attachment,
        .debugName = "depth_buffer"
    });

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        if (!currentWidth || !currentHeight) continue;
        
        // Update camera
        camera.Perspective(glm::radians(45.0f), (float)currentWidth / (float)currentHeight, 0.1f, 100.0f);
        
        // Create render pass
        const lvk::RenderPass renderPass = {
            .color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0.1f, 0.1f, 0.1f, 1.0f}}},
            .depth = {.loadOp = lvk::LoadOp_Clear, .clearDepth = 1.0f}
        };
        
        const lvk::Framebuffer framebuffer = {
            .color = {{.texture = ctx->getCurrentSwapchainTexture()}},
            .depthStencil = {.texture = depthTexture}
        };
        
        lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
        {
            cmd.cmdBeginRendering(renderPass, framebuffer);
            {
                cmd.cmdBindRenderPipeline(pipeline);
                cmd.cmdBindDepthState({.compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true});
                
                // Render all meshes
                for (const auto& mesh : meshes) {
                    cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
                    cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
                    
                    // Set up MVP matrix
                    glm::mat4 model = glm::mat4(1.0f);
                    glm::mat4 view = camera.GetViewMatrix();
                    glm::mat4 proj = camera.GetProjectionMatrix();
                    glm::mat4 mvp = proj * view * model;
                    
                    cmd.cmdPushConstants(mvp);
                    cmd.cmdDrawIndexed(mesh.indexCount);
                }
            }
            cmd.cmdEndRendering();
        }
        ctx->submit(cmd, ctx->getCurrentSwapchainTexture());
    }
    
    // Cleanup
    aiReleaseImport(scene);
    
    return 0;
}