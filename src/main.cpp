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
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <filesystem>

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

// Helper: Load texture from file
// Removed LoadTexture - using simple approach like cookbook

// Simplified vertex structure to match cookbook
using Vertex = glm::vec3; // Just position for now, like the cookbook example

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

    // Simplified approach like cookbook - just positions
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Extract positions
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        const aiVector3D v = mesh->mVertices[i];
        vertices.push_back(glm::vec3(v.x, v.y, v.z));
    }
    
    // Extract indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        for (int j = 0; j < 3; j++) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    
    out.indexCount = static_cast<uint32_t>(indices.size());
    
    // Debug: Print first few vertices
    std::cout << "Mesh vertices: ";
    for (int i = 0; i < 3 && i < vertices.size(); ++i) {
        std::cout << "(" << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << ") ";
    }
    std::cout << std::endl;

    // Create buffers exactly like cookbook
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
    
    // Load FBX asset
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    std::cout << "Looking for FBX file: assets/skull/source/skull.fbx" << std::endl;
    
    // Change to parent directory if we're in cmake-build-debug
    if (std::filesystem::current_path().filename() == "cmake-build-debug") {
        std::filesystem::current_path("..");
        std::cout << "Changed to parent directory: " << std::filesystem::current_path() << std::endl;
    }
    
    // Check if file exists
    if (!std::filesystem::exists("assets/skull/source/skull.fbx")) {
        std::cerr << "FBX file does not exist at: assets/skull/source/skull.fbx" << std::endl;
        std::cerr << "Available files in assets/skull/source/:" << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator("assets/skull/source/")) {
            std::cerr << "  " << entry.path() << std::endl;
        }
        return -1;
    }
    
    std::cout << "FBX file exists, trying to load..." << std::endl;
    
    const aiScene* scene = LoadFBX("assets/skull/source/skull.fbx");
    if (!scene) {
        std::cerr << "Failed to load FBX file. Current working directory: " << std::filesystem::current_path() << std::endl;
        return -1;
    }
    
    // Simple setup - no textures for now like cookbook

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
    
    // Create pipeline exactly like cookbook
    const lvk::VertexInput vdesc = {
        .attributes    = { { .location = 0, .format = lvk::VertexFormat::Float3, .offset = 0 } },
        .inputBindings = { { .stride = sizeof(glm::vec3) } },
    };
    
    lvk::Holder<lvk::RenderPipelineHandle> pipeline = ctx->createRenderPipeline({
        .vertexInput = vdesc,
        .smVert      = vert,
        .smFrag      = frag,
        .color       = { { .format = ctx->getSwapchainFormat() } },
        .depthFormat = lvk::Format_Z_F32,
        .cullMode    = lvk::CullMode_Back,
    });

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
        
        // Simple MVP like cookbook
        const glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        const glm::mat4 v = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -1.5f)), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
        const glm::mat4 mvp = p * v * m;
        
        const lvk::RenderPass renderPass = {
            .color = { { .loadOp = lvk::LoadOp_Clear, .clearColor = { 1.0f, 1.0f, 1.0f, 1.0f } } },
            .depth = { .loadOp = lvk::LoadOp_Clear, .clearDepth = 1.0f }
        };
        
        const lvk::Framebuffer framebuffer = {
            .color        = { { .texture = ctx->getCurrentSwapchainTexture() } },
            .depthStencil = { .texture = depthTexture },
        };
        
        lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
        {
            cmd.cmdBeginRendering(renderPass, framebuffer);
            {
                cmd.cmdBindRenderPipeline(pipeline);
                cmd.cmdBindDepthState({ .compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true });
                
                // Render all meshes
                for (const auto& mesh : meshes) {
                    cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
                    cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
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