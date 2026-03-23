#pragma once

#include <vector>

#include "protocol.h"
#include "worker.h"
#include <GLFW/glfw3.h>

#ifndef DENSITY
#define DENSITY 40
#endif

struct Color
{
    float r, g, b;
};

class renderer
{
public:
    renderer(int rank, int size);
    void render();
    void recieve();
    void mainloop();
    int isError() { return error; }
    void end();

private:
    Color HSVtoRGB(float h, float s, float v);

    int rank, size;
    GLFWwindow *window;

    std::vector<Path> paths;

    struct Scale
    {
        float scale;
        float cx;
        float cy;
    } scale;
    ViewParams params;

    int error = 0;
};