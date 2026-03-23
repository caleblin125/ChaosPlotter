#include "renderer.h"

#include <math.h>
#include <random>
#include <iostream>
#include <algorithm>
#include <mpi.h>

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

renderer::renderer(int rank, int size) : rank(rank), size(size)
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

void renderer::recieve()
{
    int flag;
    MPI_Status status;

    MPI_Iprobe(MPI_ANY_SOURCE, TAG_PATH_SIZE, MPI_COMM_WORLD, &flag, &status);

    while (flag)
    {
        MPI_Status status;
        int size;
        MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, TAG_PATH_SIZE, MPI_COMM_WORLD, &status);

        int source = status.MPI_SOURCE; // find out who sent it

        Path path(size);

        MPI_Recv(path.data(), size * 3, MPI_FLOAT, source, TAG_PATH, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        paths.push_back(path);
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_PATH_SIZE, MPI_COMM_WORLD, &flag, &status);
    }
}

void renderer::render()
{
    if (paths.size() == 0)
    {
        return;
    }

    // printf("Path size: %d \n", paths.size());

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
    float d = scale.scale / DENSITY;
    glOrtho(left, right, top, bottom, -1, 1); // ← ortho before switching
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if ((params.left != left) || (params.right != right) || (params.top != top) || (params.bottom != bottom) || (params.d != d))
    {   
        printf("Sent New Window Data\n");
        params = {left, right, top, bottom, d};
        MPI_Request bcastRequest;
        MPI_Ibcast(&params, sizeof(ViewParams), MPI_BYTE, 0, MPI_COMM_WORLD, &bcastRequest);
        MPI_Wait(&bcastRequest, MPI_STATUS_IGNORE);
    }

    // glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);

    for (Path path : paths)
    {
        for (int i = 0; i < path.size() - 1; i++)
        {
            Data p1 = path[i];
            Data p2 = path[i + 1];

            float a = atan2(p2.y - p1.y, p2.x - p1.x);
            float dist = sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));

            float p = (float)i / (float)path.size();
            Color c = HSVtoRGB(a * 360.0 / M_PI / 2.0 + 180.0, 1.0, 1.0);

            glColor4f(c.r, c.g, c.b, 0.02f * dist / 2.0);
            glVertex2f(p1.x, p1.y);
        }
    }
    paths.clear();

    glEnd();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void renderer::mainloop()
{
    printf("Started renderer mainloop\n");
    while (!glfwWindowShouldClose(window))
    {
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

        recieve();
        render();
    }

    printf("Close window\n");
    glfwTerminate();
    end();
}

void renderer::end()
{
    int endMsg = -1;
    for (int i = 1; i < size; i++)
    {
        MPI_Send(&endMsg, 1, MPI_INT, i, TAG_SHUTDOWN, MPI_COMM_WORLD);
    }
    printf("Ending\n");
}