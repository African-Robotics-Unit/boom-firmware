#include <Arduino.h>
#include <Encoder.h>
#include <IMU.h>

#define Serial Serial

// TODO
// - work out temperature conversion

Encoder pitch(1, 0);
Encoder yaw(7, 6);

IntervalTimer pllTimer;

const byte pitchIndexPin = 2;
const byte ledPin = 13;

volatile bool pitchIndexFound = false;

const float yawRadius = 2.558; // pivot to end mounting distance [m]
const float pitchRadius = 2.475; // pivot to pivot distance [m]
const uint16_t pitchIndexPos = 744;
const uint8_t gearRatio = 4;
const uint16_t CPR = 4096; // encoder counts per revolution
const uint32_t laptopBaud = 115200;

// PLL estimator stuff
// https://discourse.odriverobotics.com/t/rotor-encoder-pll-and-velocity/224
const uint16_t pllFreq = 10000; // [Hz]
const uint8_t pllBandwidth = 100; // [rad/s]
const uint8_t pll_kp = 2.0f * pllBandwidth;
const uint32_t pll_ki = 0.25f * (pll_kp * pll_kp); // critically damped
float pitch_pos_estimate = 0; // [counts]
float pitch_vel_estimate = 0; // [counts/s]
float yaw_pos_estimate = 0; // [counts]
float yaw_vel_estimate = 0; // [counts/s]

elapsedMicros loopTime; // elapsed loop time in microseconds
IMU imu;

// function declarations
void sendFloat(float);
void sendInt(uint32_t);
void pitchIndexInterrupt();
void pllLoop();
float countsToRadians(float);


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(pitchIndexPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pitchIndexPin), pitchIndexInterrupt, FALLING);
  // setup serial
  Serial.flush();
	Serial.begin(laptopBaud);
	Serial.print("Teensy comms initiated");
  // start IMU
  // need to make sure boom is not moving...
  digitalWrite(ledPin, HIGH);
  imu.start();
  digitalWrite(ledPin, LOW);
  // setup encoders
  while (!pitchIndexFound); // wait for pitch index
  // start PLL estimator
  pllTimer.begin(pllLoop, 1E6f / pllFreq);
  pllTimer.priority(0); // Highest priority. USB defaults to 112, the hardware serial ports default to 64, and systick defaults to 0.
}


// all serial communication must be done inside the main loop
void loop() {
  if (loopTime >= (2000)) { // loop must execute at 500Hz
    loopTime = 0;
    // send header
    Serial.write((uint8_t)0xAA);
    Serial.write((uint8_t)0x55);
    // calculate boom end pos and vel
    float x = countsToRadians(yaw_pos_estimate) * yawRadius; // boom end horizontal position [m]
    float dx = countsToRadians(yaw_vel_estimate) * yawRadius; // boom end horizontal speed [m/s]
    float y = countsToRadians(pitch_pos_estimate) * pitchRadius; // boom end vertical position [m]
    float dy = countsToRadians(pitch_vel_estimate) * pitchRadius; // boom end vertical speed [m/s]

    if (imu.accelAvailable()) { imu.readAccel(); }
    if (imu.tempAvailable()) { imu.readTemp(); }
    // send data
    sendFloat(x);
    sendFloat(y);
    sendFloat(dx);
    sendFloat(dy);
    sendFloat(imu.calcAccel(imu.ax)); // g's
    sendFloat(imu.calcAccel(imu.ay)); // g's
    // sendInt(imu.temperature); //
  }
}


// Sends as sequential bytes
void sendFloat(float data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}


// Sends as sequential bytes
void sendInt(int32_t data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}


// Interrupt handler for when the pitch encoder index is detected
void pitchIndexInterrupt() {
  pitchIndexFound = true;
  yaw.write(0);
  pitch.write(pitchIndexPos);
  digitalWrite(ledPin, HIGH);
  detachInterrupt(digitalPinToInterrupt(pitchIndexPin));
}


// converts encoder counts to an angle in radians
float countsToRadians(float counts) {
  return (counts / (float)(CPR * gearRatio)) * 2.0f*PI;
}


// PLL loop to estimate encoder position and velocity
// Runs at frequency defined by 'pllFreq'
void pllLoop() {
  float pll_period = 1.0f / pllFreq;
  // predicted current pos
  pitch_pos_estimate += pll_period * pitch_vel_estimate;
  yaw_pos_estimate += pll_period * yaw_vel_estimate;
  // discrete phase detector
  float pitch_delta_pos = (float)(pitch.read() - (int32_t)floor(pitch_pos_estimate));
  float yaw_delta_pos = (float)(yaw.read() - (int32_t)floor(yaw_pos_estimate));
  // pll feedback
  pitch_pos_estimate += pll_period * pll_kp * pitch_delta_pos;
  pitch_vel_estimate += pll_period * pll_ki * pitch_delta_pos;
  yaw_pos_estimate += pll_period * pll_kp * yaw_delta_pos;
  yaw_vel_estimate += pll_period * pll_ki * yaw_delta_pos;
}
