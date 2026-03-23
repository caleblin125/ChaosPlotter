#pragma once

#include <vector>

#include "protocol.h"
#include "worker.h"
#include <GLFW/glfw3.h>

struct Color
{
    double r, g, b;
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
    Color HSVtoRGB(double h, double s, double v);

    int rank, size;
    GLFWwindow *window;

    std::vector<Path> paths;

    struct Scale
    {
        double scale;
        double cx;
        double cy;
    } scale;
    ViewParams params;

    int error = 0;

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};