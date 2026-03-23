#include "function.h"

#include <math.h>

Point function(Point p, Point pi){
    float x = p.x;
    float y = p.y;
    // float xi = x*x - y + pi.x*y;
    // float yi = 2.0*y*sin(x) + pi.y*x;
    float xi = x*x - y*y + pi.x;
    float yi = 2.0*x*y + pi.y;
    return {xi, yi};
}
