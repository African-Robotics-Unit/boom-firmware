#include <Arduino.h>
#include <Encoder.h>


Encoder pitch(1, 0);
Encoder yaw(11, 10);

IntervalTimer speedTimer;

const byte pitchIndexPin = 2;
const byte ledPin = 13;

volatile bool pitchIndexFound = false;

const float yawRadius = 2.558; // pivot to end mounting distance [m]
const float pitchRadius = 2.475; // pivot to pivot distance [m]
const unsigned int gearRatio = 4.0;
const unsigned int cpr = 4000; // encoder counts per revolution
const unsigned int sampleFrequency = 1000; //Hz
const unsigned int speedUpdateFrequency = 20; //Hz higher frequency -> higher minimum speed
const unsigned int laptopBaud = 1000000;

unsigned long t = 0;

int prevYawValue = 0;
int prevPitchValue = 0;

float x = 0; // boom end horizontal position [m]
float y = 0; // boom end vertical position [m]
float dx = 0; // boom end horizontal speed [m/s]
float dy = 0; // boom end vertical speed [m/s]

// function declarations
void send(float);
void pitchIndexInterrupt();
float countsToRadians(int);
void calculatePosition();
void calculateSpeed();


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(pitchIndexPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pitchIndexPin), pitchIndexInterrupt, FALLING);

  speedTimer.begin(calculateSpeed, 1E6 / speedUpdateFrequency);

  Serial.flush();
	Serial.begin(laptopBaud);
	Serial.write("Teensy comms initiated");

  yaw.write(0);
  while (!pitchIndexFound);
  t = micros();
}

// all serial communication must be done inside the main loop
void loop() {
  t += 1E6 / sampleFrequency;
  // send header
	Serial.write((uint8_t)0xAA);
	Serial.write((uint8_t)0x55);
  calculatePosition();
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
float countsToRadians(int counts) {
  return (float(counts) / float(cpr * gearRatio)) * 2.0*PI;
}

void calculatePosition() {
  x = countsToRadians(yaw.read()) * yawRadius;
  y = countsToRadians(pitch.read()) * pitchRadius;
}

void calculateSpeed() {
  //position difference over time
  dx = countsToRadians(yaw.read() - prevYawValue) * yawRadius * speedUpdateFrequency;
  dy = countsToRadians(pitch.read() - prevPitchValue) * pitchRadius * speedUpdateFrequency;
  prevYawValue = yaw.read();
  prevPitchValue = pitch.read();
}
