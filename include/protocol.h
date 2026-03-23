#pragma once

#include <vector>

struct ViewParams
{
    float left, right, top, bottom, d;
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
    float x;
    float y;
    float f;
};

typedef std::vector<Data> Path;