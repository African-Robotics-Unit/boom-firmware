#include <Arduino.h>
#include <Encoder.h>
#include <IMU.h>
#include <PLL.h>
#include <Kalman.h>

#define Serial Serial3 // Serial3

// TODO

Encoder pitchEncoder(1, 0); // UP positive
Encoder yawEncoder(7, 6); // CCW positive
Encoder rollEncoder(10, 11); // CCW positive

IntervalTimer pllTimer;

const byte pitchIndexPin = 2;
const byte rollIndexPin = 12;
const byte ledPin = 13;
const byte transmitEnable = 9;

volatile bool pitchIndexFound = false;
volatile bool rollIndexFound = false;

const float dt = 0.001;
const float yawRadius = 2.575; // pivot to end mounting face distance [m]
const float pitchRadius = 2.493; // pivot to pivot distance [m]
const float pitchHeightOffset = 0.101; // floor to pitch pivot distance [m]
const uint16_t pitchIndexPos = 498; // calibrated with 3D printed jig and straight beam
const uint16_t rollIndexPos = 0; // calibrated with spirit level
const uint16_t pitchCPR = 4*1024*4;
const uint16_t yawCPR = 4*1024*4;
const uint16_t rollCPR = 4*500;
const uint32_t baudRate = 1e6; // [baud]

// global state variables
float x_enc = 0, y_enc = 0, r_enc = 0;
float x_kf = 0, y_kf = 0, r_pll = 0;
float dx_kf = 0, dy_kf = 0, dr_pll = 0;
float ddx_imu = 0, ddy_imu = 0, ddz_imu = 0;
int count = 0;

// PLL estimator initialisation
const float pllFreq = 20e3; // [Hz]
PLL rollPLL(1.0/pllFreq, 100);

// Kalman Filter initialisation
KalmanFilter xKF(dt, 0, 0);
KalmanFilter yKF(dt, 0.35, 0);

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
  pinMode(transmitEnable, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(pitchIndexPin, INPUT_PULLUP);
  pinMode(rollIndexPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pitchIndexPin), pitchIndexInterrupt, FALLING); // Pulse is a LOW
  attachInterrupt(digitalPinToInterrupt(rollIndexPin), rollIndexInterrupt, HIGH);
  // setup serial
  Serial.begin(baudRate);
  Serial.flush();
  // start IMU
  digitalWrite(ledPin, HIGH); // Show calibrating IMU. DO NOT MOVE END OF BOOM!
  imu.initialise();
  digitalWrite(ledPin, LOW); // Show done calibrating IMU
  // setup encoders
  while (!pitchIndexFound || !rollIndexFound); // wait for index's
  // start PLL estimator
  pllTimer.begin(updatePLL, 1e6f/pllFreq);
  pllTimer.priority(1); // High priority, USB defaults to 112, the hardware serial ports default to 64, and systick defaults to 0.
  digitalWrite(transmitEnable, HIGH); // Start sending data
  digitalWrite(ledPin, HIGH); // Show ready
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
    y_enc = pitchRadius * sin(countsToRadians(pitchEncoder.read(), pitchCPR)) + pitchHeightOffset;
    r_enc = countsToRadians(rollEncoder.read(), rollCPR);
    r_pll = countsToRadians(rollPLL.position, rollCPR);

    dr_pll = countsToRadians(rollPLL.velocity, rollCPR);

    // update kalman filter
    yKF.update(y_enc, imu.ddy);
    xKF.update(x_enc, imu.ddx);

    // get and unpack kalman filter data
    Matrix<2> X = xKF.state();
    x_kf = X(0), dx_kf = X(1);
    Matrix<2> Y = yKF.state();
    y_kf = Y(0), dy_kf = Y(1);

    // send header
    Serial.write((uint8_t)0xAA);
    Serial.write((uint8_t)0x55);
    // send data frame
    // positions
    sendFloat(x_kf);
    sendFloat(y_kf);
    sendFloat(r_pll);
    // velocities
    sendFloat(dx_kf);
    sendFloat(dy_kf);
    sendFloat(dr_pll);
    // accelerations
    sendFloat(imu.ddx);
    sendFloat(imu.ddy);
    sendFloat(imu.ddz);
    // count to see if packets are dropped
    sendFloat(count += 1);
    // sendFloat(loopTime);
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
// Will be called by default if the roll encoder is not connected
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
  rollPLL.update(rollEncoder.read());
}
