#include <stdint.h>
#include <math.h>


class PLL {

public:
    float position; // estimated position
    float velocity; // estimated velocity
    
    PLL(float, float);
    void update(int32_t);

private:
    float dt;
    float Kp;
    float Ki;
};
