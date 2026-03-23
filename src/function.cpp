#include "function.h"

#include <math.h>

Point function(Point p, Point pi){
    float x = p.x;
    float y = p.y;
    // float xi = x*x - y + pi.x*y;
    // float yi = 2.0*y*sin(x) + pi.y*x;
    float xi = x*x - y*y + pi.x;
    float yi = 2*x*y + pi.y;
    return {xi, yi};
}

std::vector<Data> compute(float x, float y){
    std::vector<Data> ret;
    Point p = {x, y};
    for(int i = 0; i < 1000; i++){
        if(p.x*p.x + p.y*p.y > 2.0){
            ret.push_back(Data{p.x, p.y, -1.0f});
            break;
        }
        
        p = function(p, {x, y});

        ret.push_back(Data{p.x, p.y, (float)i});
    }
    return ret;
}

