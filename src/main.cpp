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

std::string ReadFile(const std::filesystem::path &shader_path) {
    if (!exists(shader_path) || !is_regular_file(shader_path)) return {};

    std::ifstream file(shader_path);
    if (!file.is_open()) return {};

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char *argv[]) {

    fastgltf::Parser parser;
    std::filesystem::path path = "assets/old_rusty_car/scene.bin";
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);

    glfwInit();
    int width{800};
    int height{600};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    const lvk::Holder<lvk::ShaderModuleHandle> vert = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/basic.vert").c_str()), lvk::Stage_Vert, ("vert shader")}, nullptr);
    const lvk::Holder<lvk::ShaderModuleHandle> frag = ctx->createShaderModule(
        lvk::ShaderModuleDesc{(ReadFile("shaders/basic.frag").c_str()), lvk::Stage_Frag, ("frag shader")}, nullptr);
    const lvk::Holder<lvk::RenderPipelineHandle> pipeline = ctx->createRenderPipeline({
        .smVert = vert, .smFrag = frag, .color = {{.format = ctx->getSwapchainFormat()}}
    });
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
        command_buffer.cmdDraw(3);

        command_buffer.cmdEndRendering();
        ctx->submit(command_buffer, ctx->getCurrentSwapchainTexture());
    }
    ctx.release();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
