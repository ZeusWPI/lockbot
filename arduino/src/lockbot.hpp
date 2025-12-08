#pragma once

#include <Arduino.h>

#include "util.hpp"

// disabled until we get a 10k potentiometer
#define ENABLE_TURN_ASSIST 0

#define ENABLE_BOMBPLANTSOUND 1

#define OPEN_POS_ADDRESS 0
#define CLOSED_POS_ADDRESS 8
#define OPEN_BOUND_ADDRESS 12
#define CLOSED_BOUND_ADDRESS 16

// Connect wiper pin of 10k potentiometer to A0, the other pins to VCC and
// ground
#define POTENTIOMETER_PIN A0

// Connect IN pin of relay to pin 3, VCC pin to the motor power supply 5V,
// ground to ground common to the motor power supply 5V cable, NO (normal open)
// to the 5V cable of the motor
#define RELAY_PIN 3

// Connect one leg of the button to 5, the other to ground
#define BUTTON_PIN 5

// Leave this pin unconnected (or alternatively, connect a short length of wire
// to the pin and leave unconnected) This is used to collect randomness via ADC
// noise
#define UNCONNECTED_RANDOM_PIN A1

// SERVO_PIN_A = 9
// Connect the data pin of the servo motor to pin 9, ground to ground. VCC
// should already be connected to NO of the relay

// toneAC buzzer pins
#define BUZZER_1 4
#define BUZZER_2 7

#define ANALOG_PRECISION 4

void setup();
void loop();

void delayedLock();

void handleCommand(const char *cmd, const char *why, int *val);

LockStatus getLockStatus();

void lockDoor();
void openDoor();

void bringToState(int desired_value);

#if ENABLE_TURN_ASSIST
void turnAssistLoop();
#endif

void turnOpen();
void turnClose();
void turnHalt();

void turnDirection(bool directionIsOpen);
