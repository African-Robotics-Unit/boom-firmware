#include <stdint.h>
#include <math.h>


class PLL {

public:
    // estimated position
    float position;
    // estimated velocity
    float velocity;
    
    PLL(float dt, float bandwidth);
    
    void update(int32_t encoder_pos);

private:
    float dt;
    float Kp;
    float Ki;
};
