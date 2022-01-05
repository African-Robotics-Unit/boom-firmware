#include <PLL.h>


PLL::PLL(float dt, float bandwidth) {
    this->dt = dt;
    this->Kp = 2.0f * bandwidth;
    this->Ki = 0.25f * (Kp * Kp);
    this->position = 0;
    this->velocity = 0;
}


// Must be called at a fixed frequency
// https://discourse.odriverobotics.com/t/rotor-encoder-pll-and-velocity/224
void PLL::update(int32_t encoder_pos) {
    position += dt * velocity;
    float delta_pos = (float)(encoder_pos - (int32_t)floor(position));
    position += dt * Kp * delta_pos;
    velocity += dt * Ki * delta_pos;
}
