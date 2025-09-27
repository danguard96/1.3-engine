//
// Created by Yibuz Pokopodrozo on 2025-09-16.
//

#include <filesystem>
#include <iostream>
#include <GLFW/glfw3.h>
#include <lightweightvk/lvk/LVK.h>
#include <fstream>
#include <sstream>
#include <fastgltf/core.hpp>
#include "gltf_loader.h"
#include <optional>

std::string ReadFile(const std::filesystem::path &shader_path) {
    if (!exists(shader_path) || !is_regular_file(shader_path)) return {};

    std::ifstream file(shader_path);
    if (!file.is_open()) return {};

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char *argv[]) {
    glfwInit();
    int width{800};
    int height{600};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    
    // Load glTF asset
    GltfLoader loader(ctx.get());
    std::filesystem::path gltfPath = "assets/old_rusty_car/scene.gltf";
    auto gltfAsset = loader.loadGltf(gltfPath);
    
    if (!gltfAsset.has_value()) {
        std::cerr << "Failed to load glTF asset: " << gltfPath << std::endl;
        return -1;
    }
    
    std::cout << "Successfully loaded glTF asset with " << gltfAsset->meshes.size() << " meshes" << std::endl;
    const lvk::Holder<lvk::ShaderModuleHandle> vert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/basic.vert").c_str()), lvk::Stage_Vert, ("vert shader")}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> frag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/basic.frag").c_str()), lvk::Stage_Frag, ("frag shader")}, nullptr);
    lvk::RenderPipelineDesc pipelineDesc{};
    pipelineDesc.smVert = vert;
    pipelineDesc.smFrag = frag;
    pipelineDesc.color[0].format = ctx->getSwapchainFormat();
    
    // Set up vertex input
    pipelineDesc.vertexInput.attributes[0] = {.location = 0, .binding = 0, .format = lvk::VertexFormat::Float3, .offset = 0};  // position
    pipelineDesc.vertexInput.attributes[1] = {.location = 1, .binding = 0, .format = lvk::VertexFormat::Float3, .offset = 12}; // normal
    pipelineDesc.vertexInput.attributes[2] = {.location = 2, .binding = 0, .format = lvk::VertexFormat::Float2, .offset = 24}; // texCoord
    pipelineDesc.vertexInput.attributes[3] = {.location = 3, .binding = 0, .format = lvk::VertexFormat::Float4, .offset = 32}; // tangent
    pipelineDesc.vertexInput.inputBindings[0] = {.stride = 48};
    
    const lvk::Holder<lvk::RenderPipelineHandle> pipeline = ctx->createRenderPipeline(pipelineDesc);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwGetFramebufferSize(window, &width, &height);
        if (!width || !height) {
            continue;
        }
        lvk::ICommandBuffer &command_buffer = ctx->acquireCommandBuffer();
        command_buffer.cmdBeginRendering({.color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0, 0, 0, 1}}}},
                                         {.color = {{.texture = ctx->getCurrentSwapchainTexture()}}});
        command_buffer.cmdBindRenderPipeline(pipeline);
        
        // Render all meshes from the glTF asset
        for (size_t i = 0; i < gltfAsset->meshes.size(); ++i) {
            const auto& mesh = gltfAsset->meshes[i];
            if (i < gltfAsset->vertexBuffers.size() && i < gltfAsset->indexBuffers.size()) {
                // Bind vertex buffer
                command_buffer.cmdBindVertexBuffer(0, gltfAsset->vertexBuffers[i]);
                // Bind index buffer and draw
                command_buffer.cmdBindIndexBuffer(gltfAsset->indexBuffers[i], lvk::IndexFormat_UI32);
                command_buffer.cmdDrawIndexed(mesh.indices.size());
            }
        }

        command_buffer.cmdEndRendering();
        ctx->submit(command_buffer, ctx->getCurrentSwapchainTexture());
    }
    ctx.release();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
