#include <kalman.h>


KalmanFilter::KalmanFilter(float dt, Matrix<3> x0) {
    dt = dt;
    x = x0;
    P = { 1, 0, 0,
        0, 1, 0,
        0, 0, 1 }; 
    F = { 1, dt, 0.5*dt*dt,
        0, 1, dt,
        0, 0, 1  };
    Gamma = { 0.5*dt*dt, dt, 1 };
    float j = 5; // jerk
    Q = Gamma * j*j*dt*dt * ~Gamma; //fix this
    H = { 1, 0, 0,
        0, 0, 1 };
    R = { 1e-12, 0,
        0, 2e-3 };
}


void KalmanFilter::update(const Matrix<2>& z) {
    // predict
    x = F * x;
    P = F * P * ~F + Q;
    // update
    S = H * P * ~H + R;
    S_inv = S;
    Invert(S_inv);
    K = P * ~H * S_inv;
    x += K * (z - H * x);
    P = P - K * H * P;
}
