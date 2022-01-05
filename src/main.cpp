#include <Arduino.h>
#include <Encoder.h>
#include <IMU.h>
#include <PLL.h>
#include <Kalman.h>

#define Serial Serial1 // Serial1

// TODO
// - Check roll encoder direction
// - Check roll velocity estimation
// - Does IndexFound need to be volatile?

Encoder pitchEncoder(1, 0);
Encoder yawEncoder(6, 7); // CW direction positive (based on IMU z axis direction)
Encoder rollEncoder(10, 11); // CCW positive

IntervalTimer pllTimer;

const byte pitchIndexPin = 2;
const byte rollIndexPin = 12;
const byte ledPin = 13;
const byte transmitEnable = 14;

volatile bool pitchIndexFound = false;
volatile bool rollIndexFound = false;

const float dt = 0.001;
const float yawRadius = 2.558; // pivot to end mounting distance [m]
const float pitchRadius = 2.475; // pivot to pivot distance [m]
const uint16_t pitchIndexPos = 658; // for calibrating the pitch axis
const uint16_t rollIndexPos = 0; // for calibrating the roll axis
const uint16_t pitchCPR = 4*1024*4;
const uint16_t yawCPR = 4*1024*4;
const uint16_t rollCPR = 4*500;
const uint32_t baudRate = 1e6; // [baud]

// global state variables
float x_enc = 0, y_enc = 0, r_enc = 0;
float x_pll = 0, y_pll = 0, r_pll = 0;
float x_kf = 0, y_kf = 0;
float dx_pll = 0, dy_pll = 0, dr_pll = 0;
float dx_kf = 0, dy_kf = 0;
float ddx_imu = 0, ddy_imu = 0, ddz_imu = 0;
float ddx_kf = 0, ddy_kf = 0;

// PLL estimator initialisation
const float pllFreq = 20e3; // [Hz]
PLL pitchPLL(1.0/pllFreq, 500);
PLL yawPLL(1.0/pllFreq, 500);
PLL rollPLL(1.0/pllFreq, 1000);

// Kalman Filter initialisation
Matrix<3> X_initial = {0, 0, 0};
KalmanFilter xKF(dt, X_initial);
Matrix<3> Y_initial = {0.35, 0, 0};
KalmanFilter yKF(dt, Y_initial);

elapsedMicros loopTime; // elapsed loop time in microseconds
IMU imu;

// function declarations
void sendFloat(float);
void sendInt(int32_t);
void pitchIndexInterrupt();
void rollIndexInterrupt();
void updatePLL();
float countsToRadians(float, int);


void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(transmitEnable, OUTPUT); // Only transmitting
  digitalWrite(transmitEnable, HIGH);
  digitalWrite(ledPin, LOW);
  pinMode(pitchIndexPin, INPUT_PULLUP);
  pinMode(rollIndexPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pitchIndexPin), pitchIndexInterrupt, HIGH);
  attachInterrupt(digitalPinToInterrupt(rollIndexPin), rollIndexInterrupt, HIGH);
  // setup serial
  Serial1.setTX(5); // Using alternate Serial1 TX
  Serial1.setRX(21); // Using alternate Serial1 RX
  Serial.begin(baudRate);
  Serial.flush();
  // start IMU
  digitalWrite(ledPin, HIGH);
  imu.initialise();
  digitalWrite(ledPin, LOW);
  // setup encoders
  while (!pitchIndexFound || !rollIndexFound); // wait for index's
  digitalWrite(ledPin, HIGH); // Encoders calibrated
  // start PLL estimator
  pllTimer.begin(updatePLL, 1e6f/pllFreq);
  pllTimer.priority(0); // Highest priority. USB defaults to 112, the hardware serial ports default to 64, and systick defaults to 0.
}


// all serial communication must be done inside the main loop
void loop() {
  if (loopTime >= (1e6f * dt)) {
    loopTime = 0;
    // read data from accelerometer when available
    imu.readAcceleration();
    imu.readTemperature();
    // calculate boom end pos and vel
    x_enc = yawRadius * countsToRadians(yawEncoder.read(), yawCPR);
    x_pll = yawRadius * countsToRadians(yawPLL.position, yawCPR); // boom end horizontal position [m]
    y_enc = pitchRadius * sin(countsToRadians(pitchEncoder.read(), pitchCPR));
    y_pll = pitchRadius * sin(countsToRadians(pitchPLL.position, pitchCPR)); // boom end vertical position [m]
    r_enc = countsToRadians(rollEncoder.read(), rollCPR);
    r_pll = countsToRadians(rollPLL.position, rollCPR);

    dx_pll = yawRadius * countsToRadians(yawPLL.velocity, yawCPR); // boom end horizontal speed [m/s]
    dy_pll = pitchRadius * cos(countsToRadians(pitchPLL.position, pitchCPR)) * countsToRadians(pitchPLL.velocity, pitchCPR); // boom end vertical speed [m/s]
    dr_pll = countsToRadians(rollPLL.velocity, rollCPR);

    // update kalman filter
    yKF.update((Matrix<2>){y_enc, imu.ddy});
    xKF.update((Matrix<2>){x_enc, imu.ddx});

    // get and unpack kalman filter data
    Matrix<3> X = xKF.state();
    x_kf = X(0), dx_kf = X(1), ddx_kf = X(2);
    Matrix<3> Y = yKF.state();
    y_kf = Y(0), dy_kf = Y(1), ddy_kf = Y(2);

    // send header
    Serial.write((uint8_t)0xAA);
    Serial.write((uint8_t)0x55);
    // send data frame
    sendFloat(x_enc);
    sendFloat(x_pll);
    sendFloat(x_kf);
    sendFloat(y_enc);
    sendFloat(y_pll);
    sendFloat(y_kf);
    sendFloat(r_enc);

    sendFloat(dx_pll);
    sendFloat(dx_kf);
    sendFloat(dy_pll);
    sendFloat(dy_kf);
    sendFloat(dr_pll);

    sendFloat(imu.ddx);
    sendFloat(imu.ddy);
    sendFloat(imu.ddz);
    // sendFloat(ddx_kf);
    // sendFloat(ddy_kf);
    sendFloat(imu.temperatureCelcius);
  }
}


void sendFloat(float data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}

void sendInt(int32_t data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}


// Interrupt handler for when the pitch index is detected
void pitchIndexInterrupt() {
  pitchIndexFound = true;
  yawEncoder.write(0);
  pitchEncoder.write(pitchIndexPos);
  detachInterrupt(digitalPinToInterrupt(pitchIndexPin));
}

// Interrupt handler for when the roll index is detected
// Will be called even if the roll encoder is not connected
void rollIndexInterrupt() {
  rollIndexFound = true;
  rollEncoder.write(rollIndexPos);
  detachInterrupt(digitalPinToInterrupt(rollIndexPin));
}

// converts encoder counts to an angle in radians
float countsToRadians(float counts, int CPR) {
  return (counts / (float)CPR) * 2.0f*PI;
}

// PLL loop to estimate encoder position and velocity
// Runs at frequency defined by 'pllFreq'
void updatePLL() {
  pitchPLL.update(pitchEncoder.read());
  yawPLL.update(yawEncoder.read());
  rollPLL.update(rollEncoder.read());
}
