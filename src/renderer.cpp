#include "renderer.h"

#include <math.h>
#include <random>
#include <iostream>
#include <algorithm>
#include "function.h"

// h: 0-360, s: 0-1, v: 0-1
Color renderer::HSVtoRGB(float h, float s, float v)
{
    float c = v * s; // Chroma
    float x = c * (1 - std::fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float r1, g1, b1;

    if (h < 60)
    {
        r1 = c;
        g1 = x;
        b1 = 0;
    }
    else if (h < 120)
    {
        r1 = x;
        g1 = c;
        b1 = 0;
    }
    else if (h < 180)
    {
        r1 = 0;
        g1 = c;
        b1 = x;
    }
    else if (h < 240)
    {
        r1 = 0;
        g1 = x;
        b1 = c;
    }
    else if (h < 300)
    {
        r1 = x;
        g1 = 0;
        b1 = c;
    }
    else
    {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    return {r1 + m, g1 + m, b1 + m};
}

renderer::renderer()
{
    glfwInit();
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        std::cerr << "Failed to get primary monitor\n";
        error = 1;
        return;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
    {
        std::cerr << "Failed to get video mode\n";
        error = 1;
        return;
    }

    // Create fullscreen window using the monitor's current resolution
    window = glfwCreateWindow(
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
        error = 1;
        return;
    }

    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // this is default, but explicit

    glPointSize(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    scale.scale = 1.5f;
    scale.cx = -0.5f;
    scale.cy = 0.0f;
}

int renderer::shouldClose()
{
    return glfwWindowShouldClose(window);
}

void renderer::render()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float bound = std::min(width, height);
    float widthf = (float)width;
    float heightf = (float)height;

    float left = -widthf / bound * scale.scale + scale.cx;
    float right = widthf / bound * scale.scale + scale.cx;
    float top = -heightf / bound * scale.scale + scale.cy;
    float bottom = heightf / bound * scale.scale + scale.cy;
    glOrtho(left, right, top, bottom, -1, 1); // ← ortho before switching
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // glClear(GL_COLOR_BUFFER_BIT);

    // Press Escape to exit fullscreen
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        // get monitor dimensions for windowed mode position
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        int w = 800, h = 600;
        glfwSetWindowMonitor(window, NULL,
                             (mode->width - w) / 2,  // centered X
                             (mode->height - h) / 2, // centered Y
                             w, h, 0);
    }

    glBegin(GL_POINTS);

    float n = 40;
    float d = scale.scale / n;
    for (float cx = left; cx <= right; cx += d)
    {
        for (float cy = top; cy <= bottom; cy += d)
        {
            float dx = d * ((float)random() / (float)RAND_MAX - 0.5);
            float dy = d * ((float)random() / (float)RAND_MAX - 0.5);

            float xi = cx + dx;
            float yi = cy + dy;
            std::vector<Data> orbit = compute(xi, yi);
            if (orbit.size() == 0)
            {
                continue;
            }
            if (orbit.back().f < 0)
            {
                continue;
            }
            for (int i = 0; i < orbit.size() - 1; i++)
            {
                Data p1 = orbit[i];
                Data p2 = orbit[i + 1];

                float a = atan2(p2.y - p1.y, p2.x - p1.x);
                float dist = sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));

                float p = (float)i / (float)orbit.size();
                Color c = HSVtoRGB(a * 360.0 / M_PI / 2.0 + 180.0, 1.0, 1.0);

                glColor4f(c.r, c.g, c.b, 0.02f * dist / 2.0);
                glVertex2f(p1.x, p1.y);
            }
        }
    }

    glEnd();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void renderer::mainloop()
{
    while (!shouldClose())
    {
        render();
    }
    glfwTerminate();
}