#pragma once

#include <vector>

struct Data{
    float x;
    float y;
    float f;
};

struct Point{
    float x;
    float y;
};

Point function(Point p, Point pi);

std::vector<Data> compute(float x, float y);