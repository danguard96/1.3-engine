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

// Component system includes
#include <components/Actor.h>
#include <components/TransformComponent.h>
#include <components/MeshComponent.h>
#include <components/CameraComponent.h>

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

// Helper function for texture loading (still needed for rendering)
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

// MeshBuffers struct is now defined in MeshComponent.h
// UploadMesh function is now part of MeshComponent class

int main(int argc, char *argv[]) {
    glfwInit();
    int width{800};
    int height{600};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    
    // Enable cursor capture for mouse look
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    
    // Simple setup like cookbook
    
    // Change to parent directory if we're in cmake-build-debug
    if (std::filesystem::current_path().filename() == "cmake-build-debug") {
        std::filesystem::current_path("..");
    }
    
    // Load texture for rendering
    auto skullColor = LoadTexture(ctx.get(), "assets/skull/textures/skullColor.png");
    
    // Component System Example
    std::cout << "\n=== Component System Demo ===" << std::endl;
    
    // Create a skull actor with components
    Actor* skullActor = new Actor();
    
    // Add transform component
    skullActor->AddComponent<TransformComponent>(skullActor, 
        glm::vec3(0.0f, 0.5f, 0.0f),  // Position
        glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),  // 90° X-axis rotation
        glm::vec3(1.0f, 1.0f, 1.0f)   // Scale
    );
    
    // Add mesh component
    skullActor->AddComponent<MeshComponent>(skullActor, ctx.get(), "assets/skull/source/skull.fbx");
    
    // Initialize the actor
    if (!skullActor->OnCreate()) {
        std::cerr << "Failed to create skull actor" << std::endl;
        return -1;
    }
    
    // Create a second actor (another skull) next to the first one
    Actor* skullActor2 = new Actor();
    
    // Add transform component for the second skull
    skullActor2->AddComponent<TransformComponent>(skullActor2, 
        glm::vec3(0.5f, 0.0f, 0.0f),  // Position - to the right of first skull
        glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),  // Same rotation
        glm::vec3(1.0f, 1.0f, 1.0f)   // Scale
    );
    
    // Add mesh component for the second skull
    skullActor2->AddComponent<MeshComponent>(skullActor2, ctx.get(), "assets/skull/source/skull.fbx");
    
    // Initialize the second actor
    if (!skullActor2->OnCreate()) {
        std::cerr << "Failed to create second skull actor" << std::endl;
        return -1;
    }
    
    // List components for both actors
    std::cout << "\n=== First Skull Actor ===" << std::endl;
    skullActor->ListComponents();
    
    std::cout << "\n=== Second Skull Actor ===" << std::endl;
    skullActor2->ListComponents();
    
    // Get transform component and modify it
    TransformComponent* transform = skullActor->GetComponent<TransformComponent>();
    if (transform) {
        std::cout << "Initial position: " << transform->GetPosition().x << ", " 
                  << transform->GetPosition().y << ", " << transform->GetPosition().z << std::endl;
        
        // Keep skull at original position
        // transform->SetPosition(glm::vec3(0.0f, 0.1f, 0.0f));
        
        std::cout << "New position: " << transform->GetPosition().x << ", " 
                  << transform->GetPosition().y << ", " << transform->GetPosition().z << std::endl;
    }
    
    // Second skull stays at its initial position
    // TransformComponent* transform2 = skullActor2->GetComponent<TransformComponent>();
    // if (transform2) {
    //     transform2->SetPosition(glm::vec3(0.5f, 0.1f, 0.0f));  // Same Y offset as first skull
    // }
    
    // Get mesh component
    MeshComponent* meshComp = skullActor->GetComponent<MeshComponent>();
    if (meshComp) {
        std::cout << "Mesh component has " << meshComp->GetMeshes().size() << " meshes" << std::endl;
    }
    
    std::cout << "=== Component System Demo Complete ===\n" << std::endl;

    // Create camera actor
    Actor* cameraActor = new Actor();
    cameraActor->AddComponent<CameraComponent>(cameraActor, 45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    if (!cameraActor->OnCreate()) {
        std::cerr << "Failed to create camera actor" << std::endl;
        return -1;
    }
    
    // Get camera component
    CameraComponent* camera = cameraActor->GetComponent<CameraComponent>();
    if (camera) {
        camera->SetLookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),  // Eye position
            glm::vec3(0.0f, 0.0f, 0.0f),  // Look at center
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
        );
        std::cout << "Camera created at position: " << camera->GetPosition().x << ", " 
                  << camera->GetPosition().y << ", " << camera->GetPosition().z << std::endl;
        
        // Test: Force camera to move after 3 seconds
        std::cout << "Camera will auto-move in 3 seconds for testing..." << std::endl;
    } else {
        std::cout << "ERROR: Failed to get camera component!" << std::endl;
    }

    // Get meshes from the component system
    if (!meshComp) {
        std::cerr << "No mesh component found!" << std::endl;
        return -1;
    }
    const std::vector<MeshBuffers>& meshes = meshComp->GetMeshes();

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
    // Delta time tracking
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        if (!currentWidth || !currentHeight) continue;
        
        const float ratio = currentWidth / (float)currentHeight;
        
        // Update camera with new aspect ratio
        if (camera) {
            camera->SetPerspective(45.0f, ratio, 0.1f, 1000.0f);
            
            // Test: Auto-move camera after 3 seconds
            static double startTime = glfwGetTime();
            if (glfwGetTime() - startTime > 3.0) {
                static bool autoMoved = false;
                if (!autoMoved) {
                    std::cout << "Auto-moving camera for test..." << std::endl;
                    camera->SetPosition(glm::vec3(2.0f, 1.0f, 3.0f));
                    camera->SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
                    autoMoved = true;
                }
            }
            
            camera->Update(deltaTime);
            camera->HandleInput(deltaTime, window);
            
            // Debug: Print camera position every 60 frames (about once per second)
            static int frameCount = 0;
            if (++frameCount % 60 == 0) {
                std::cout << "Camera position: " << camera->GetPosition().x << ", " 
                          << camera->GetPosition().y << ", " << camera->GetPosition().z << std::endl;
            }
        } else {
            static bool cameraNullWarning = false;
            if (!cameraNullWarning) {
                std::cout << "WARNING: Camera is null!" << std::endl;
                cameraNullWarning = true;
            }
        }
        
        // Get camera matrices
        const glm::mat4 v = camera ? camera->GetViewMatrix() : glm::mat4(1.0f);
        const glm::mat4 p = camera ? camera->GetProjectionMatrix() : glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
        
        
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
                
                // Define push constants struct once
                struct PushConstants {
                    glm::mat4 mvp;
                    glm::mat4 model;
                    uint32_t textureIndex;
                };
                
                // Render first skull actor (no rotation)
                const glm::mat4 m1 = skullActor->GetModelMatrix();
                const glm::mat4 mvp1 = p * v * m1;
                PushConstants pushConstants1 = { 
                    mvp1, m1, skullColor.index()
                };
                cmd.cmdPushConstants(pushConstants1);
                
                for (const auto& mesh : meshes) {
                    cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
                    cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
                    cmd.cmdDrawIndexed(mesh.indexCount);
                }
                
                // Render second skull actor
                const glm::mat4 m2 = skullActor2->GetModelMatrix();
                const glm::mat4 mvp2 = p * v * m2;
                PushConstants pushConstants2 = { 
                    mvp2, m2, skullColor.index()
                };
                cmd.cmdPushConstants(pushConstants2);
                
                // Get meshes from second actor
                MeshComponent* meshComp2 = skullActor2->GetComponent<MeshComponent>();
                if (meshComp2) {
                    const std::vector<MeshBuffers>& meshes2 = meshComp2->GetMeshes();
                    for (const auto& mesh : meshes2) {
                        cmd.cmdBindVertexBuffer(0, mesh.vertexBuffer);
                        cmd.cmdBindIndexBuffer(mesh.indexBuffer, lvk::IndexFormat_UI32);
                        cmd.cmdDrawIndexed(mesh.indexCount);
                    }
                }
            }
            cmd.cmdEndRendering();
        }
        ctx->submit(cmd, ctx->getCurrentSwapchainTexture());
    }
    
    // Cleanup component system
    if (skullActor) {
        skullActor->OnDestroy();
        delete skullActor;
    }
    
    if (skullActor2) {
        skullActor2->OnDestroy();
        delete skullActor2;
    }
    
    if (cameraActor) {
        cameraActor->OnDestroy();
        delete cameraActor;
    }
    
    return 0;
}