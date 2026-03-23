#pragma once

#include <GLFW/glfw3.h>


struct Color {
    float r, g, b;
};

void renderInit(GLFWwindow* window);
void render(GLFWwindow* window);

Color HSVtoRGB(float h, float s, float v);