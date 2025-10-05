//
// Created by Yibuz Pokopodrozo on 2025-09-16.
//

#include <lvk/LVK.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <memory>
#include <stdexcept>

std::string ReadFile(const std::filesystem::path& shader_path) {
    if (!std::filesystem::exists(shader_path) || !std::filesystem::is_regular_file(shader_path)) {
        return {};
    }

    std::ifstream file(shader_path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    // Reserve space for better performance
    file.seekg(0, std::ios::end);
    const auto file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content;
    content.reserve(static_cast<size_t>(file_size));
    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    
    return content;
}

// Helper: Load FBX model using assimp
const aiScene* LoadFBX(const char* path) {
    std::cout << "Attempting to load FBX file: " << path << std::endl;
    
    const aiScene* scene = aiImportFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    
    if (!scene) {
        std::cerr << "Failed to load FBX: " << path << "\n";
        std::cerr << "Assimp error: " << aiGetErrorString() << std::endl;
        return nullptr;
    }
    
    std::cout << "FBX loaded successfully. Meshes: " << scene->mNumMeshes << std::endl;
    
    if (!scene->HasMeshes()) {
        std::cerr << "No meshes found in FBX file: " << path << "\n";
        aiReleaseImport(scene);
        return nullptr;
    }
    
    return scene;
}

// Helper: Load texture from file with error handling
lvk::Holder<lvk::TextureHandle> LoadTexture(lvk::IContext* ctx, const char* fileName) {
    int width, height, channels;
    unsigned char* data = stbi_load(fileName, &width, &height, &channels, 4); // Force RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << fileName << std::endl;
        return {};
    }
    
    auto texture = ctx->createTexture({
        .type = lvk::TextureType_2D,
        .format = lvk::Format_RGBA_SRGB8, // Use sRGB for color textures
        .dimensions = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = lvk::TextureUsageBits_Sampled,
        .data = data,
        .debugName = fileName
    });
    
    stbi_image_free(data);
    return texture;
}

// Vertex structure with position, normal, and texture coordinates
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
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

MeshBuffers UploadMesh(lvk::IContext* ctx, const aiMesh* mesh) {
    MeshBuffers out{};

    if (!mesh->HasPositions()) {
        throw std::runtime_error("Mesh has no positions");
    }

    // Extract vertices with position, normal, and texture coordinates
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Extract vertex data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Position
        const aiVector3D pos = mesh->mVertices[i];
        vertex.position = glm::vec3(pos.x, pos.y, pos.z);
        
        // Normal
        if (mesh->HasNormals()) {
            const aiVector3D norm = mesh->mNormals[i];
            vertex.normal = glm::vec3(norm.x, norm.y, norm.z);
    } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up normal
        }
        
        // Texture coordinates
        if (mesh->HasTextureCoords(0)) {
            const aiVector3D tex = mesh->mTextureCoords[0][i];
            vertex.texCoord = glm::vec2(tex.x, tex.y);
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f); // Default UV
        }
        
        vertices.push_back(vertex);
    }
    
    // Extract indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        for (int j = 0; j < 3; j++) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    
    out.indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers with optimized data upload
    out.vertexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Vertex,
        .storage = lvk::StorageType_Device,
        .size = sizeof(Vertex) * vertices.size(),
        .data = vertices.data(),
        .debugName = "Buffer: vertex"
    });
    
    out.indexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Index,
        .storage = lvk::StorageType_Device,
        .size = sizeof(uint32_t) * indices.size(),
        .data = indices.data(),
        .debugName = "Buffer: index"
    });

    return out;
}

int main(int argc, char *argv[]) {
    glfwInit();
    int width{800};
    int height{600};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    
    // Simple setup like cookbook
    
    // Change to parent directory if we're in cmake-build-debug
    if (std::filesystem::current_path().filename() == "cmake-build-debug") {
        std::filesystem::current_path("..");
    }
    
    // Load FBX asset
    const aiScene* scene = LoadFBX("assets/skull/source/skull.fbx");
    if (!scene) {
        std::cerr << "Failed to load FBX file" << std::endl;
        return -1;
    }
    
    // Load textures for the skull model
    auto skullColor = LoadTexture(ctx.get(), "assets/skull/textures/skullColor.png");

    std::vector<MeshBuffers> meshes;
    meshes.reserve(scene->mNumMeshes); // Reserve space for better performance
    
    for (size_t mi = 0; mi < scene->mNumMeshes; ++mi) {
            try {
            meshes.emplace_back(UploadMesh(ctx.get(), scene->mMeshes[mi]));
            } catch (const std::exception& e) {
            std::cerr << "Failed to upload mesh " << mi << ": " << e.what() << std::endl;
        }
    }

    // Create shaders with cached source
    const std::string vertSource = ReadFile("shaders/blinn_phong.vert");
    const std::string fragSource = ReadFile("shaders/blinn_phong.frag");
    
    const lvk::Holder<lvk::ShaderModuleHandle> vert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{vertSource.c_str(), lvk::Stage_Vert, "vert shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> frag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{fragSource.c_str(), lvk::Stage_Frag, "frag shader"}, nullptr);
    
    // Create pipeline with texture support
    const lvk::VertexInput vdesc = {
        .attributes    = { 
            { .location = 0, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, position) },
            { .location = 1, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, normal) },
            { .location = 2, .format = lvk::VertexFormat::Float2, .offset = offsetof(Vertex, texCoord) }
        },
        .inputBindings = { { .stride = sizeof(Vertex) } },
    };
    
    lvk::Holder<lvk::RenderPipelineHandle> pipeline = ctx->createRenderPipeline({
        .vertexInput = vdesc,
        .smVert      = vert,
        .smFrag      = frag,
        .color       = { { .format = ctx->getSwapchainFormat() } },
        .depthFormat = lvk::Format_Z_F32,
        .cullMode    = lvk::CullMode_Back,
    });
    
    // Create sampler for textures
    lvk::Holder<lvk::SamplerHandle> sampler = ctx->createSampler({
        .minFilter = lvk::SamplerFilter::SamplerFilter_Linear,
        .magFilter = lvk::SamplerFilter::SamplerFilter_Linear,
        .mipMap    = lvk::SamplerMip::SamplerMip_Linear,
        .wrapU     = lvk::SamplerWrap::SamplerWrap_Repeat,
        .wrapV     = lvk::SamplerWrap::SamplerWrap_Repeat,
        .wrapW     = lvk::SamplerWrap::SamplerWrap_Repeat,
        .debugName = "Texture Sampler",
    });

    // We'll use simple push constants for now

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
        
        const float ratio = currentWidth / (float)currentHeight;
        
        // Simple MVP like cookbook - adjusted for better viewing
        const glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        // Move camera closer and position skull higher in the window
        const glm::mat4 v = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f)), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
        const glm::mat4 mvp = p * v * m;
        
        const lvk::RenderPass renderPass = {
            .color = { { .loadOp = lvk::LoadOp_Clear, .clearColor = { 0.2f, 0.3f, 0.4f, 1.0f } } }, // Dark greyish blue
            .depth = { .loadOp = lvk::LoadOp_Clear, .clearDepth = 1.0f }
        };
        
        const lvk::Framebuffer framebuffer = {
            .color        = { { .texture = ctx->getCurrentSwapchainTexture() } },
            .depthStencil = { .texture = depthTexture },
        };
        
        lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
        {
            // Pass texture as dependency so LightweightVK can bind it to descriptor sets
            cmd.cmdBeginRendering(renderPass, framebuffer, { 
                .textures = { skullColor }
            });
            {
                cmd.cmdBindRenderPipeline(pipeline);
                cmd.cmdBindDepthState({ .compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true });
                
                // Use push constants with texture index
                struct PushConstants {
                    glm::mat4 mvp;
                    uint32_t textureIndex;
                } pushConstants = { mvp, skullColor.index() };
                cmd.cmdPushConstants(pushConstants);
                
                // Render all meshes
                for (const auto& mesh : meshes) {
                    cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
                    cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
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