
#include "function.h"

#include <math.h>

std::vector<Data> compute(float x, float y){
    std::vector<Data> ret;
    float xi = x, yi = y;
    for(int i = 0; i < 100; i++){
        if(xi*xi + yi*yi > 4){
            break;
        }
        float xit = xi, yit = yi;

        xi = xit*xit - yit*yit + x;
        yi = 2*xit*yit + y;
        
        ret.push_back(Data{xi, yi, (float)i});
    }
    return ret;
}