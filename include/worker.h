#pragma once

#include <vector>
#include "protocol.h"

#ifndef ITERATIONS
#define ITERATIONS 1000
#endif

#ifndef BOUND
#define BOUND 4.0f
#endif

class worker
{
public:
    worker(int rank, int size);
    Path computePath(float x, float y);
    void compute();
    void recieve();
    void mainloop();

private:
    int rank, size;
    ViewParams window;

    bool endWorker = false;
};