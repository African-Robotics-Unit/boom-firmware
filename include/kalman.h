#include <BasicLinearAlgebra.h>

using namespace BLA;

class KalmanFilter {

public:
    KalmanFilter(float dt, Matrix<3> x0);

    /**
     * Update the estimated state based on measured values
     */
    void update(const Matrix<2>& z);

    /**
     * Return the current state
     */
    Matrix<3> state() { return x; };

private:
    static const int n = 3;
    static const int m = 2;

    Matrix<n> x; // predicted state
    Matrix<n,n> P; // state covariance
    Matrix<n,n> F; // process function
    Matrix<n> Gamma;
    Matrix<n,n> Q; // process covariance
    Matrix<m,n> H; // measurement function
    Matrix<m,m> R; // measurement covariance

    Matrix<m,m> S; // intermediate matrix
    Matrix<m,m> S_inv; // intermediate inverse matrix
    Matrix<n,m> K; // kalman gain

    // Discrete time step
    float dt;
};
