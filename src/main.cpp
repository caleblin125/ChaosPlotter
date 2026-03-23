
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>

#include "renderer.h"

int main()
{

    if (!glfwInit())
        return -1;

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        std::cerr << "Failed to get primary monitor\n";
        return -1;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
    {
        std::cerr << "Failed to get video mode\n";
        return -1;
    }

    // Create fullscreen window using the monitor's current resolution
    GLFWwindow *window = glfwCreateWindow(
        mode->width,
        mode->height,
        "ChaosPlotter",
        monitor, // fullscreen
        nullptr  // no shared context
    );
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    renderInit(window);
    while (!glfwWindowShouldClose(window))
    {
        render(window);
    }

    glfwTerminate();
    return 0;
}