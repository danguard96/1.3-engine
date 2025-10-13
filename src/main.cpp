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
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <memory>

// Component system includes
#include <components/Actor.h>
#include <components/TransformComponent.h>
#include <components/MeshComponent.h>
#include <components/CameraComponent.h>

// ImGui includes
#include <imgui.h>
#include <lvk/HelpersImGui.h>
#include <float.h> // For FLT_MAX

std::string ReadFile(const std::filesystem::path& shader_path) {
    if (!std::filesystem::exists(shader_path) || !std::filesystem::is_regular_file(shader_path)) {
        return {};
    }

    std::ifstream file(shader_path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

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
    int width{0};
    int height{0};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    
    // Initialize ImGui exactly like the cookbook
    std::unique_ptr<lvk::ImGuiRenderer> imgui = std::make_unique<lvk::ImGuiRenderer>(*ctx);
    
    // GLFW input callbacks exactly like the cookbook
    glfwSetCursorPosCallback(window, [](auto* window, double x, double y) { 
        ImGui::GetIO().MousePos = ImVec2(x, y); 
    });
    glfwSetMouseButtonCallback(window, [](auto* window, int button, int action, int mods) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
                                                  ? ImGuiMouseButton_Left
                                                  : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2((float)xpos, (float)ypos);
        io.MouseDown[imguiButton] = action == GLFW_PRESS;
    });
    glfwSetKeyCallback(window, [](auto* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });
    
    // Simple setup like cookbook
    
    // Change to parent directory if we're in cmake-build-debug
    if (std::filesystem::current_path().filename() == "cmake-build-debug") {
        std::filesystem::current_path("..");
    }
    
    // Load texture for rendering
    auto skullColor = LoadTexture(ctx.get(), "assets/skull/textures/skullColor.png");
    if (!skullColor.valid()) {
        std::cerr << "ERROR: Failed to load skull texture!" << std::endl;
        return -1;
    }
    std::cout << "Skull texture loaded successfully, index: " << skullColor.index() << std::endl;
    
    // Load noise texture for fog effects
    auto noise = LoadTexture(ctx.get(), "assets/noise/512x512/Super Perlin/Super Perlin 9 - 512x512.png");
    if (!noise.valid()) {
        std::cerr << "ERROR: Failed to load Gabor noise texture!" << std::endl;
        return -1;
    }
    std::cout << "Gabor noise texture loaded successfully, index: " << noise.index() << std::endl;

    auto noise2 = LoadTexture(ctx.get(), "assets/noise/512x512/Swirl/Swirl 6 - 512x512.png");
    if (!noise2.valid()) {
        std::cerr << "ERROR: Failed to load Gabor noise texture!" << std::endl;
        return -1;
    }
    std::cout << "Gabor noise texture loaded successfully, index: " << noise2.index() << std::endl;
    
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
        
        std::cout << "New position: " << transform->GetPosition().x << ", " 
                  << transform->GetPosition().y << ", " << transform->GetPosition().z << std::endl;
    }
    
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

    // Create main rendering shaders
    const std::string vertSource = ReadFile("shaders/blinn_phong.vert");
    const std::string fragSource = ReadFile("shaders/blinn_phong.frag");
    
    const lvk::Holder<lvk::ShaderModuleHandle> vert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{vertSource.c_str(), lvk::Stage_Vert, "vert shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> frag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{fragSource.c_str(), lvk::Stage_Frag, "frag shader"}, nullptr);
    
    // Load post-processing shaders
    const std::string postVertSource = ReadFile("shaders/post.vert");
    const lvk::Holder<lvk::ShaderModuleHandle> postVert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{postVertSource.c_str(), lvk::Stage_Vert, "post vert shader"}, nullptr);
    
    // Load all post-processing fragment shaders
    const std::string nopostFragSource = ReadFile("shaders/nopost.frag");
    const std::string crtFragSource = ReadFile("shaders/CRT-dynamic.frag");
    const std::string bloomFragSource = ReadFile("shaders/bloom.frag");
    const std::string dreamFragSource = ReadFile("shaders/dream.frag");
    const std::string glitchFragSource = ReadFile("shaders/glitch.frag");
    const std::string pixelFragSource = ReadFile("shaders/pixelation.frag");
    const std::string fogFragSource = ReadFile("shaders/fog.frag");
    const std::string underwaterFragSource = ReadFile("shaders/underwater.frag");
    const std::string ditheringFragSource = ReadFile("shaders/dithering.frag");
    const std::string posterizationFragSource = ReadFile("shaders/posterization.frag");

    const lvk::Holder<lvk::ShaderModuleHandle> nopostFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{nopostFragSource.c_str(), lvk::Stage_Frag, "nopost frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> crtFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{crtFragSource.c_str(), lvk::Stage_Frag, "crt frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> bloomFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{bloomFragSource.c_str(), lvk::Stage_Frag, "bloom frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> dreamFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{dreamFragSource.c_str(), lvk::Stage_Frag, "dream frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> glitchFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{glitchFragSource.c_str(), lvk::Stage_Frag, "glitch frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> pixelFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{pixelFragSource.c_str(), lvk::Stage_Frag, "pixel frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> fogFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{fogFragSource.c_str(), lvk::Stage_Frag, "fog frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> underwaterFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{underwaterFragSource.c_str(), lvk::Stage_Frag, "underwater frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> ditheringFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{ditheringFragSource.c_str(), lvk::Stage_Frag, "dithering frag shader"}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> posterizationFrag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{posterizationFragSource.c_str(), lvk::Stage_Frag, "posterization frag shader"}, nullptr);
    
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
        .debugName   = "Main Pipeline",
    });
    
    // Create post-processing pipelines following cookbook pattern
    lvk::Holder<lvk::RenderPipelineHandle> pipelineToneMap = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = nopostFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });
    
    lvk::Holder<lvk::RenderPipelineHandle> pipelineCRT = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = crtFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });
    
    lvk::Holder<lvk::RenderPipelineHandle> pipelineBloom = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = bloomFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });
    
    lvk::Holder<lvk::RenderPipelineHandle> pipelineDream = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = dreamFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });
    
    lvk::Holder<lvk::RenderPipelineHandle> pipelineGlitch = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = glitchFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });
    
    lvk::Holder<lvk::RenderPipelineHandle> pipelinePixel = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = pixelFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });

    lvk::Holder<lvk::RenderPipelineHandle> pipelineFog = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = fogFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });

    lvk::Holder<lvk::RenderPipelineHandle> pipelineUnderwater = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = underwaterFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });

    lvk::Holder<lvk::RenderPipelineHandle> pipelineDithering = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = ditheringFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });

    lvk::Holder<lvk::RenderPipelineHandle> pipelinePosterization = ctx->createRenderPipeline({
        .smVert = postVert,
        .smFrag = posterizationFrag,
        .color  = { { .format = ctx->getSwapchainFormat() } },
    });
    
    // Create intermediate framebuffer for post-processing (following cookbook pattern)
    const lvk::Dimensions sizeFb = ctx->getDimensions(ctx->getCurrentSwapchainTexture());
    
    lvk::Holder<lvk::TextureHandle> intermediateTexture = ctx->createTexture({
        .format     = ctx->getSwapchainFormat(),
        .dimensions = sizeFb,
        .usage      = lvk::TextureUsageBits_Attachment | lvk::TextureUsageBits_Sampled,
        .debugName  = "Intermediate Texture",
    });
    
    lvk::Holder<lvk::TextureHandle> intermediateDepth = ctx->createTexture({
        .format     = lvk::Format_Z_F32,
        .dimensions = sizeFb,
        .usage      = lvk::TextureUsageBits_Attachment,
        .debugName  = "Intermediate Depth",
    });
    
    // Create sampler for textures (following cookbook pattern)
    lvk::Holder<lvk::SamplerHandle> sampler = ctx->createSampler({
        .wrapU = lvk::SamplerWrap_Clamp,
        .wrapV = lvk::SamplerWrap_Clamp,
        .wrapW = lvk::SamplerWrap_Clamp,
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
        
        // Post-processing effect selection
        static int currentEffect = 0;
        
        lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
        {
            // Render main scene to intermediate framebuffer
            const lvk::RenderPass renderPassOffscreen = {
                .color = { { .loadOp = lvk::LoadOp_Clear, .clearColor = { 0.2f, 0.3f, 0.4f, 1.0f } } },
                .depth = { .loadOp = lvk::LoadOp_Clear, .clearDepth = 1.0f }
            };
            
            const lvk::Framebuffer framebufferOffscreen = {
                .color        = { { .texture = intermediateTexture } },
                .depthStencil = { .texture = intermediateDepth },
            };
            
            cmd.cmdBeginRendering(renderPassOffscreen, framebufferOffscreen, { 
                .textures = { skullColor }
            });
            
            {
                cmd.cmdBindRenderPipeline(pipeline);
                cmd.cmdBindDepthState({ .compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true });
                
                struct PushConstants {
                    glm::mat4 mvp;
                    glm::mat4 model;
                    uint32_t textureIndex;
                    float _padding[3]; // Ensure 16-byte alignment
                };
                
                // Render first skull actor
                const glm::mat4 m1 = skullActor->GetModelMatrix();
                const glm::mat4 mvp1 = p * v * m1;
                PushConstants pushConstants1 = { 
                    mvp1, m1, skullColor.index(), {0.0f, 0.0f, 0.0f}
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
                    mvp2, m2, skullColor.index(), {0.0f, 0.0f, 0.0f}
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
            
            // Apply post-processing effects to final framebuffer
            const lvk::RenderPass renderPassMain = {
                .color = { { .loadOp = lvk::LoadOp_Clear, .clearColor = { 1.0f, 1.0f, 1.0f, 1.0f } } },
            };
            
            const lvk::Framebuffer framebufferMain = {
                .color = { { .texture = ctx->getCurrentSwapchainTexture() } },
            };
            
            cmd.cmdBeginRendering(renderPassMain, framebufferMain, { 
                .textures = { lvk::TextureHandle(intermediateTexture), noise, noise2 }
            });
            
            // Select post-processing pipeline based on current effect
            lvk::Holder<lvk::RenderPipelineHandle>* selectedPipeline;
            switch (currentEffect) {
                case 0: selectedPipeline = &pipelineToneMap; break;
                case 1: selectedPipeline = &pipelineCRT; break;
                case 2: selectedPipeline = &pipelineBloom; break;
                case 3: selectedPipeline = &pipelineDream; break;
                case 4: selectedPipeline = &pipelineGlitch; break;
                case 5: selectedPipeline = &pipelinePixel; break;
                case 6: selectedPipeline = &pipelineFog; break;
                case 7: selectedPipeline = &pipelineUnderwater; break;
                case 8: selectedPipeline = &pipelineDithering; break;
                case 9: selectedPipeline = &pipelinePosterization; break;
                default: selectedPipeline = &pipelineToneMap; break;
            }
            
            cmd.cmdBindRenderPipeline(*selectedPipeline);
            cmd.cmdBindDepthState({});
            
            // Push constants for post-processing
            struct PostPushConstants {
                uint32_t texColor;
                uint32_t smpl;
                float time;
                uint32_t noise;
                uint32_t noise2;
            };
            
            PostPushConstants postPush = { 
                intermediateTexture.index(), 
                sampler.index(), 
                static_cast<float>(currentTime),
                noise.index(),
                noise2.index()
            };
            cmd.cmdPushConstants(postPush);
            
            // Render fullscreen triangle
            cmd.cmdDraw(3);
            
            // Render ImGui on top
            imgui->beginFrame(framebufferMain);
            
            // Post-processing control overlay
            ImGui::Begin("Post-Processing Effects", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Select Post-Processing Effect:");
            if (ImGui::Button("No Post-Processing")) currentEffect = 0;
            if (ImGui::Button("CRT Dynamic")) currentEffect = 1;
            if (ImGui::Button("Bloom")) currentEffect = 2;
            if (ImGui::Button("Dream")) currentEffect = 3;
            if (ImGui::Button("Glitch")) currentEffect = 4;
            if (ImGui::Button("Pixelation")) currentEffect = 5;
            if (ImGui::Button("Fog")) currentEffect = 6;
            if (ImGui::Button("Underwater")) currentEffect = 7;
            if (ImGui::Button("Dithering")) currentEffect = 8;
            if (ImGui::Button("Posterization")) currentEffect = 9;
            ImGui::End();
            
            imgui->endFrame(cmd);
            
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