#pragma once

#include <GLFW/glfw3.h>

struct Color
{
    float r, g, b;
};

class renderer
{
public:
    renderer();
    int shouldClose();
    void render();
    void mainloop();
    int isError() { return error; }

private:
    Color HSVtoRGB(float h, float s, float v);

    GLFWwindow *window;

    struct Scale
    {
        float scale;
        float cx;
        float cy;
    } scale;

    int error = 0;
};