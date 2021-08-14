#include <Arduino.h>
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>


Encoder pitch(1, 0);
Encoder yaw(11, 10);

IntervalTimer pllTimer;

const byte pitchIndexPin = 2;
const byte ledPin = 13;

volatile bool pitchIndexFound = false;

const float yawRadius = 2.558; // pivot to end mounting distance [m]
const float pitchRadius = 2.475; // pivot to pivot distance [m]
const uint16_t pitchIndexPos = 671;
const uint8_t gearRatio = 4;
const uint16_t CPR = 4096; // encoder counts per revolution
const uint16_t serialFreq = 1000; // [Hz] This might need to be lower
const uint32_t laptopBaud = 1000000;

// PLL estimator stuff
// https://discourse.odriverobotics.com/t/rotor-encoder-pll-and-velocity/224
const uint16_t pllFreq = 10000; // [Hz]
const uint8_t pllBandwidth = 100; // [rad/s]
const uint16_t pll_kp = 2.0f * pllBandwidth;
const uint16_t pll_ki = 0.25f * (pll_kp * pll_kp); // critically damped
float pitch_pos_estimate = 0; // [counts]
float pitch_vel_estimate = 0; // [counts/s]
float yaw_pos_estimate = 0; // [counts]
float yaw_vel_estimate = 0; // [counts/s]

uint32_t t = 0; // time variable for managing main loop frequency
elapsedMicros loopTime; // elapsed loop time in microseconds

Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x28);

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
  // setup encoders
  yaw.write(0);
  // while (!pitchIndexFound); // wait for pitch index
  // start PLL estimator
  pllTimer.begin(pllLoop, 1E6f / pllFreq);
  pllTimer.priority(0); // Highest priority. USB defaults to 112, the hardware serial ports default to 64, and systick defaults to 0.
  // setup IMU
  if(!bno.begin()) {
    Serial.print("Could not connect to BNO055 IMU");
    while(1);
  }
  bno.setExtCrystalUse(true);
  Wire.setClock(3200000); // 32kHz high speed mode
}

// all serial communication must be done inside the main loop
void loop() {
  if (loopTime >= (100)) { // loop must last more than 100us
    loopTime = 0;
    // send header
    Serial.write((uint8_t)0xAA);
    Serial.write((uint8_t)0x55);
    // calculate boom end pos and vel
    float x = countsToRadians(yaw_pos_estimate) * yawRadius; // boom end horizontal position [m]
    float y = countsToRadians(pitch_pos_estimate) * pitchRadius; // boom end vertical position [m]
    float dx = countsToRadians(yaw_vel_estimate) * yawRadius; // boom end horizontal speed [m/s]
    float dy = countsToRadians(pitch_vel_estimate) * pitchRadius; // boom end vertical speed [m/s]

    // get IMU data
    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER); // [m/s^2]

    // send data
    sendInt(yaw.read());
    sendInt(pitch.read());
    sendFloat(x);
    sendFloat(y);
    sendFloat(dx);
    sendFloat(dy);
    sendFloat(accel.x());
    sendFloat(accel.y());
    sendFloat(accel.z());
  }
}


// Sends as sequential bytes
void sendFloat(float data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}

// Sends as sequential bytes
void sendInt(uint32_t data) {
	Serial.write((uint8_t *)&data, sizeof(data));
}

// Interrupt handler for when the pitch encoder index is found
void pitchIndexInterrupt() {
  pitchIndexFound = true;
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
