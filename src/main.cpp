#include <Arduino.h>
#include <Encoder.h>
#include <IMU.h>
#include <PLL.h>
#include <Kalman.h>

#define Serial Serial

// TODO
// - work out temperature conversion

Encoder pitchEncoder(1, 0);
Encoder yawEncoder(7, 6);

IntervalTimer pllTimer;
IntervalTimer kfTimer;

const byte pitchIndexPin = 2;
const byte ledPin = 13;
const byte DE = 12;

volatile bool pitchIndexFound = false;

const float dt = 0.002;
const float yawRadius = 2.558; // pivot to end mounting distance [m]
const float pitchRadius = 2.475; // pivot to pivot distance [m]
const uint16_t pitchIndexPos = 658; // for calibrating the pitch axis
const uint8_t gearRatio = 4;
const uint16_t CPR = 4096; // encoder counts per revolution
const uint32_t laptopBaud = 1e6; // [baud]

// global state variables
float x_enc = 0, y_enc = 0;
float x_pll = 0, y_pll = 0;
float x_kf = 0, y_kf = 0;
float dx_pll = 0, dy_pll = 0;
float dx_kf = 0, dy_kf = 0;
float ddx_imu = 0, ddy_imu = 0, ddz_imu = 0;
float ddx_kf = 0, ddy_kf = 0;

// PLL estimator initialisation
const float pllFreq = 20000; // [Hz]
PLL pitchPLL(1.0/pllFreq, 500);
PLL yawPLL(1.0/pllFreq, 500);

// Kalman Filter initialisation
const float kfFreq = 1000; // [Hz]
Matrix<3> x0 = {0, 0, 0};
KalmanFilter xKF(1.0/pllFreq, x0);
Matrix<3> y0 = {0.35, 0, 0};
KalmanFilter yKF(1.0/pllFreq, y0);

elapsedMicros loopTime; // elapsed loop time in microseconds
IMU imu;

// function declarations
void sendFloat(float);
void sendInt(int32_t);
void pitchIndexInterrupt();
void updatePLL();
void updateKF();
float countsToRadians(float);


void setup() {
  pinMode(DE, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(DE, HIGH); // Enable transmit on RS485 transciever
  digitalWrite(ledPin, LOW);
  pinMode(pitchIndexPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pitchIndexPin), pitchIndexInterrupt, FALLING);
  // setup serial
  Serial.flush();
	Serial.begin(laptopBaud);
	Serial.print("Teensy comms initiated");
  // start IMU
  digitalWrite(ledPin, HIGH);
  imu.initialise();
  digitalWrite(ledPin, LOW);
  // setup encoders
  while (!pitchIndexFound); // wait for pitch index
  // start PLL estimator
  pllTimer.begin(updatePLL, 1e6f/pllFreq);
  pllTimer.priority(1); // Highest priority. USB defaults to 112, the hardware serial ports default to 64, and systick defaults to 0.
  kfTimer.begin(updateKF, 1e6f/kfFreq);
  kfTimer.priority(2);
}


// all serial communication must be done inside the main loop
void loop() {
  if (loopTime >= (1e6f * dt)) { // 500Hz
    loopTime = 0;
    // read data from accelerometer when available
    if (imu.tempAvailable()) { imu.readTemp(); }
    // calculate boom end pos and vel
    dx_pll = yawRadius * countsToRadians(yawPLL.velocity); // boom end horizontal speed [m/s]
    dy_pll = pitchRadius * cos(countsToRadians(pitchPLL.position)) * countsToRadians(pitchPLL.velocity); // boom end vertical speed [m/s]
    int32_t temp = imu.temperature; // unknown units

    // get and unpack kalman filter data
    Matrix<3> X = xKF.state();
    x_kf = X(0), dx_kf = X(1), dx_kf = X(2);
    Matrix<3> Y = yKF.state();
    y_kf = Y(0), dy_kf = Y(1), dy_kf = Y(2);

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
    sendFloat(dx_pll);
    sendFloat(dx_kf);
    sendFloat(dy_pll);
    sendFloat(dy_kf);
    sendFloat(ddx_imu);
    sendFloat(ddx_kf);
    sendFloat(ddy_imu);
    sendFloat(ddy_kf);
    sendFloat(ddz_imu);
    sendInt(temp);
  }
}


void sendFloat(float data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}

void sendInt(int32_t data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}

// Interrupt handler for when the pitch encoder index is detected
void pitchIndexInterrupt() {
  pitchIndexFound = true;
  yawEncoder.write(0);
  pitchEncoder.write(pitchIndexPos);
  digitalWrite(ledPin, HIGH);
  detachInterrupt(digitalPinToInterrupt(pitchIndexPin));
}

// converts encoder counts to an angle in radians
float countsToRadians(float counts) {
  return (counts / (float)(CPR * gearRatio)) * 2.0f*PI;
}

// PLL loop to estimate encoder position and velocity
// Runs at frequency defined by 'pllFreq'
void updatePLL() {
  pitchPLL.update(pitchEncoder.read());
  yawPLL.update(yawEncoder.read());
}

// Runs at 1kHz
void updateKF() {
  // get latest acceleration values
  // IMU ODR is only 952Hz
  imu.readAcceleration();
  // assign state data to variables
  x_enc = yawRadius * countsToRadians(yawEncoder.read());
  x_pll = yawRadius * countsToRadians(yawPLL.position); // boom end horizontal position [m]
  y_enc = pitchRadius * sin(countsToRadians(pitchEncoder.read()));
  y_pll = pitchRadius * sin(countsToRadians(pitchPLL.position)); // boom end vertical position [m]
  ddx_imu = imu.ddx;
  ddy_imu = imu.ddy;
  ddz_imu = imu.ddz;
  // update kalman filter
  yKF.update((Matrix<2>){y_pll, ddy_imu});
  xKF.update((Matrix<2>){x_pll, ddx_imu});
}
