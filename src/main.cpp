#include <Arduino.h>
#include <Encoder.h>


Encoder pitch(1, 0);
Encoder yaw(11, 10);

IntervalTimer pllTimer;

const byte pitchIndexPin = 2;
const byte ledPin = 13;

volatile bool pitchIndexFound = false;

const float yawRadius = 2.558; // pivot to end mounting distance [m]
const float pitchRadius = 2.475; // pivot to pivot distance [m]
const unsigned int gearRatio = 4.0;
const unsigned int CPR = 4000; // encoder counts per revolution
const unsigned int serialFreq = 1000; // [Hz]
const unsigned int laptopBaud = 1000000;

// PLL estimator stuff
// https://discourse.odriverobotics.com/t/rotor-encoder-pll-and-velocity/224
const unsigned int pllFreq = 10000; // [Hz] not sure what this should be
const unsigned int pllBandwidth = 1000; // [rad/s] ODrive default
const float pll_kp = 2.0 * pllBandwidth;
const float pll_ki = 0.25 * (pll_kp * pll_kp); // critically damped
float pitch_pos_estimate = 0; // [counts]
float pitch_vel_estimate = 0; // [counts/s]
float yaw_pos_estimate = 0; // [counts]
float yaw_vel_estimate = 0; // [counts/s]

unsigned long t = 0;

// function declarations
void send(float);
void pitchIndexInterrupt();
float countsToRadians(float);
void pllLoop();


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(pitchIndexPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pitchIndexPin), pitchIndexInterrupt, FALLING);

  Serial.flush();
	Serial.begin(laptopBaud);
	Serial.write("Teensy comms initiated");

  yaw.write(0);
  while (!pitchIndexFound);
  pllTimer.begin(pllLoop, 1E6 / pllFreq);
  t = micros();
}

// all serial communication must be done inside the main loop
void loop() {
  t += 1E6 / serialFreq;
  // send header
	Serial.write((uint8_t)0xAA);
	Serial.write((uint8_t)0x55);
  // calculate boom end pos and vel
  float x = countsToRadians(yaw_pos_estimate) * yawRadius; // boom end horizontal position [m]
  float y = countsToRadians(pitch_pos_estimate) * pitchRadius; // boom end vertical position [m]
  float dx = countsToRadians(yaw_vel_estimate) * yawRadius; // boom end horizontal speed [m/s]
  float dy = countsToRadians(pitch_vel_estimate) * pitchRadius; // boom end vertical speed [m/s]
  // send data
  send(x);
  send(y);
  send(dx);
  send(dy);
  // wait to ensure fixed sample frequency
  while(t > micros());
}


// Sends as sequential bytes
void send(float data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}

// Interrupt handler for when the pitch encoder index is found
void pitchIndexInterrupt() {
  pitchIndexFound = true;
  pitch.write(671); // update this to recalibrate the boom height
  digitalWrite(ledPin, HIGH);
  detachInterrupt(digitalPinToInterrupt(pitchIndexPin));
}

// converts encoder counts to an angle in radians
float countsToRadians(float counts) {
  return (counts / (float)(CPR * gearRatio)) * 2.0f*PI;
}

// PLL loop
// Runs at frequency defined by 'pllFreq'
void pllLoop() {
  float pll_period = 1.0 / pllFreq;
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
