//
// Created by Yibuz Pokopodrozo on 2025-09-16.
//

#include <iostream>
#include <GLFW/glfw3.h>
#include <lightweightvk/lvk/LVK.h>

int main (int argc, char *argv[]) {
    glfwInit();
    int width {800};
    int height {600};
    GLFWwindow *window = lvk::initWindow("VKEngine", width, height, false);
    std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "Hello World!\n";
}
