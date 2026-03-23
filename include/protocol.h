#pragma once

#include <vector>

#ifndef ITERATIONS
#define ITERATIONS 1000
#endif

#ifndef BOUND
#define BOUND 40000.0f
#endif

#ifndef DENSITY
#define DENSITY 40
#endif

struct ViewParams
{
    double left, right, top, bottom, d;
};

enum Tags
{
    TAG_SHUTDOWN,
    TAG_PATH_SIZE,
    TAG_PATH,
    TAG_VIEWING
};

struct Data
{
    double x;
    double y;
    double f;
};

typedef std::vector<Data> Path;