#include "renderer.h"

#include <math.h>
#include <random>
#include <iostream>
#include "function.h"

// h: 0-360, s: 0-1, v: 0-1
Color HSVtoRGB(float h, float s, float v) {
    float c = v * s; // Chroma
    float x = c * (1 - std::fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float r1, g1, b1;

    if (h < 60)      { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120){ r1 = x; g1 = c; b1 = 0; }
    else if (h < 180){ r1 = 0; g1 = c; b1 = x; }
    else if (h < 240){ r1 = 0; g1 = x; b1 = c; }
    else if (h < 300){ r1 = x; g1 = 0; b1 = c; }
    else             { r1 = c; g1 = 0; b1 = x; }

    return { r1 + m, g1 + m, b1 + m };
}

void renderInit(GLFWwindow* window){
    glPointSize(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render(GLFWwindow* window){
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);

    float d = 0.05f;
    for (float cx = -2.0f; cx <= 2.0f; cx += d)
    {
        for (float cy = -2.0f; cy <= 2.0f; cy += d)
        {
            float dx = d * ((float)random() / (float)INT32_MAX - 0.5);
            float dy = d * ((float)random() / (float)INT32_MAX - 0.5);

            float xi = cx+dx;
            float yi = cy+dy;
            std::vector<Data> orbit = compute(xi, yi);
            if(orbit.size() == 0){
                continue;
            }
            if(orbit.back().f < 0){
                continue;
            }
            for (int i = 0; i < orbit.size() - 1; i++)
            {
                Data p1 = orbit[i];
                Data p2 = orbit[i+1];
                float sx = (p1.x + 2.5f) / 4.0f * width;
                float sy = (p1.y + 2.0f) / 4.0f * height;
                
                float a = atan2(p2.y-p1.y, p2.x-p1.x);
                float d = sqrt((p2.y-p1.y)*(p2.y-p1.y) + (p2.x-p1.x)*(p2.x-p1.x));

                float p = (float)i / (float)orbit.size();
                Color c = HSVtoRGB(a*360.0/M_PI/2.0 + 180.0, 1.0, 1.0);

                glColor4f(c.r, c.g, c.b, 0.10f*p*(d/4.0f));
                glVertex2f(sx, sy);
            }
        }
    }
    glEnd();

    glfwSwapBuffers(window);
    glfwPollEvents();
}