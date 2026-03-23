#include "renderer.h"

#include <math.h>
#include <random>
#include <iostream>
#include <algorithm>
#include <mpi.h>
#include <chrono>
#include <cstring>

// h: 0-360, s: 0-1, v: 0-1
Color renderer::HSVtoRGB(double h, double s, double v)
{
    double c = v * s; // Chroma
    double x = c * (1 - std::fabs(fmod(h / 60.0f, 2) - 1));
    double m = v - c;

    double r1, g1, b1;

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


void renderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    renderer* r = static_cast<renderer*>(glfwGetWindowUserPointer(window));
    if (r) {
        r->scale.scale *= exp(-yoffset*0.05);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

renderer::renderer(int rank, int size) : rank(rank), size(size)
{
    glfwInit();
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        printf("Failed to get primary monitor\n");
        error = 1;
        return;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
    {
        printf("Failed to get video mode\n");
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

    // window = glfwCreateWindow(
    //     mode->width / 2,
    //     mode->height / 2,
    //     "ChaosPlotter",
    //     nullptr, // fullscreen
    //     nullptr  // no shared context
    // );

    if (!window)
    {
        printf("Failed to create GLFW window\n");
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

    scale.scale = 0.7;
    scale.cx = 0.0;
    scale.cy = 0.0;

    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, scroll_callback);
}

void renderer::recieve()
{
    int flag;
    MPI_Status status;

    MPI_Iprobe(MPI_ANY_SOURCE, TAG_PATH_SIZE, MPI_COMM_WORLD, &flag, &status);

    while (flag && (paths.size() < 100))
    {
        int source = status.MPI_SOURCE;
        int pathCount, batchSize;
        MPI_Recv(&batchSize, 1, MPI_INT, source, TAG_PATH_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<double> batch(batchSize);
        MPI_Recv(batch.data(), batchSize, MPI_DOUBLE, source, TAG_PATH, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // unpack
        int i = 0;
        double* raw = batch.data();
        while (i < batchSize) {
            int sz = (int)raw[i++];

            Path path(sz);
            memcpy(path.data(), raw + i, sz * sizeof(Data));
            i += sz * 3;

            paths.push_back(std::move(path));
        }

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

    double bound = std::min(width, height);
    double widthf = (double)width;
    double heightf = (double)height;

    double left = -widthf / bound * scale.scale + scale.cx;
    double right = widthf / bound * scale.scale + scale.cx;
    double top = -heightf / bound * scale.scale + scale.cy;
    double bottom = heightf / bound * scale.scale + scale.cy;
    double d = scale.scale / (double)DENSITY;
    glOrtho(left, right, top, bottom, -1, 1); // ← ortho before switching
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if ((params.left != left) || (params.right != right) || (params.top != top) || (params.bottom != bottom) || (params.d != d))
    {   
        // printf("Sent New Window Data: scale: %e center: (%e, %e)\n", scale.scale, scale.cx, scale.cy);
        params = {left, right, top, bottom, d};
        for (int i = 1; i < size; i++){
            MPI_Send(&params, sizeof(ViewParams), MPI_BYTE, i, TAG_VIEWING, MPI_COMM_WORLD);
        }
    }

    // glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);

    for (Path path : paths)
    {

        //Colorful 
        for (int i = 0; i < path.size() - 1; i++)
        {
            Data p1 = path[i];
            Data p2 = path[i + 1];

            double a = atan2(p2.y - p1.y, p2.x - p1.x);
            double dist = sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));

            double p = (double)i / (double)path.size();
            Color c = HSVtoRGB(a * 360.0 / M_PI / 2.0 + 180.0, 1.0, 1.0);

            // glColor4f(c.r, c.g, c.b, 0.02f * dist / 2.0f);
            glColor4f(c.r, c.g, c.b, 0.02f);
            glVertex2f(p1.x, p1.y);

        }

        //Original mandelbrot style
        if(path[0].f == ITERATIONS){
            glColor4f(1.0, 1.0, 1.0, 1.0);
            glVertex2f(path[0].x, path[0].y);
        }
        else{
            double r = fmod(path[0].f, 5.0f)  /  5.0;
            double g = fmod(path[0].f, 7.0f)  /  7.0;
            double b = fmod(path[0].f, 19.0f) / 19.0;
            glColor4f(r, g, b, 1.0);
            glVertex2f(path[0].x, path[0].y);
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

    auto start = std::chrono::steady_clock::now();
    auto prevT = std::chrono::steady_clock::now();
    int total = 0;
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

        double dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - prevT).count();
        prevT = std::chrono::steady_clock::now();
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            scale.cy += scale.scale * 0.5 * dt;
            glClear(GL_COLOR_BUFFER_BIT);
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            // get monitor dimensions for windowed mode position
            scale.cy -= scale.scale * 0.5 * dt;
            glClear(GL_COLOR_BUFFER_BIT);
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            scale.cx -= scale.scale * 0.5 * dt;
            glClear(GL_COLOR_BUFFER_BIT);
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            scale.cx += scale.scale * 0.5*dt;
            glClear(GL_COLOR_BUFFER_BIT);
        }

        recieve();

        if(paths.size() > 0){
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            total += paths.size();
            printf("Paths per second: %lf, total paths %d \n", (double)total / elapsed, total);
        }

        render();
    }

    printf("Close window\n");
    glfwTerminate();
    end();
}

void renderer::end()
{
    printf("Sending shutdown to %d workers\n", size - 1);
    int endMsg = -1;
    for (int i = 1; i < size; i++)
    {
        MPI_Send(&endMsg, 1, MPI_INT, i, TAG_SHUTDOWN, MPI_COMM_WORLD);
        printf("Sent shutdown to worker %d\n", i);

        int flag;
        MPI_Status status;

        MPI_Iprobe(i, TAG_SHUTDOWN, MPI_COMM_WORLD, &flag, &status);
        while (!flag)
        {
            recieve();
            MPI_Iprobe(i, TAG_SHUTDOWN, MPI_COMM_WORLD, &flag, &status);
            // printf("Awaiting %d\n", i);
        }
        int endPong;
        MPI_Recv(&endPong, 1, MPI_INT, i, TAG_SHUTDOWN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Shutdown worker %d\n", i);
    }
    printf("Ending\n");
}