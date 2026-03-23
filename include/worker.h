#pragma once

#include <vector>
#include "protocol.h"

class worker
{
public:
    worker(int rank, int size);
    Path computePath(double x, double y);
    void compute();
    void recieve();
    void mainloop();

private:
    int rank, size;
    ViewParams window;

    bool endWorker = false;
};