#pragma once

#include <vector>
#include "protocol.h"

class worker
{
public:
    worker(int rank);
    Path computePath(float x, float y);
    void compute();
    void recieve();
    void mainloop();

private:
    int rank;
    ViewParams window;

    bool endWorker = false;
};