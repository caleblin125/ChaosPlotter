#include "function.h"

#include <math.h>

Point function(Point p, Point pi){
    double x = p.x;
    double y = p.y;

    // double tx = x*x - y*y + pi.x;
    // double ty = 2*x*y + pi.y;
    // double xi = tx*x - abs(ty*y - pi.x);
    // double yi = abs(tx*y + ty*x) + pi.y;

    //Burning ship
    // double xi = x*x - y*y + pi.x;
    // double yi = abs(2*x*y) + pi.y;

    //Mandelbrot
    double xi = x*x - y*y + pi.x;
    double yi = 2.0*x*y + pi.y;
    return {xi, yi};
}
