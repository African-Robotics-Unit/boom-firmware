#include <kalman.h>


KalmanFilter::KalmanFilter(float dt, float x0, float dx0) {
    dt = dt;
	x = {x0, dx0};
    P = {   1, 0,
            0, 1	}; 
    F = {   1, dt,
            0, 1	};
	B = {	0.5*dt*dt,
			dt	};
    Gamma = { dt, 1 };
    Q = Gamma * 2e-4 * ~Gamma;
	H = {   1, 0	};
    R = {   1e-4	};
}


void KalmanFilter::update(float enc, float imu) {
	Matrix<1> z = {enc};
	Matrix<1> u = {imu};
    // predict
    x = F * x + B * u;
    P = F * P * ~F + Q;
    // update
    S = H * P * ~H + R;
    S_inv = S;
    Invert(S_inv);
    K = P * ~H * S_inv;
    x += K * (z - H * x);
    P = P - K * H * P;
}
