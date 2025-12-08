#include "lockbot.hpp"

#include <Arduino.h>
#include <EEPROM.h>

#include <PWMServo.h>

#include <toneAC2.h>

#include "ethernet.hpp"
#include "http_server.hpp"
#include "mattermore_http_client.hpp"
#include "strings.hpp"
#include "util.hpp"

PWMServo door{};
#if ENABLE_TURN_ASSIST
int turnAssistLastValue = 0;
uint32_t turnAssistLastMove = 0;
#endif

Debounced delayedLockButton{};

HttpServer httpServer(80);

char randomGeneratedChars[32 + 1]{};

LockStatus lastStatus{};

uint32_t lastPotValuePrintTimestamp = 0;
uint32_t lastStateChangedTimestamp = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initial beep
  toneAC2(BUZZER_1, BUZZER_2, 500);

#if ENABLE_TURN_ASSIST
  turnAssistLastValue = analogRead(POTENTIOMETER_PIN);
#endif

  Serial.begin(9600);
  Serial.println(LOG_BOOTING);

  Serial.println(LOG_INITIALIZING_SERVO);
  // Config servo motor control pin, 500-2500 is for the 10kg/cm servo
  // SERVO_PIN_A = pin 9 see http://arduiniana.org/libraries/pwmservo/
  door.attach(SERVO_PIN_A);
  turnHalt();

  Serial.println(LOG_INITIALIZING_ETHERNET);
  setupEthernet();

  Serial.println(LOG_STARTING_WEBSERVER);
  httpServer.start();

  noToneAC2();

  mattermoreHttpPost(COMMAND_UP, REASON_BOOT, getLockStatus());

  for (size_t i = 0; i < sizeof(randomGeneratedChars) - 1; i++) {
    randomGeneratedChars[i] = 'A' + (analogRead(UNCONNECTED_RANDOM_PIN) % 26);
  }

  lastStatus = getLockStatus();
  mattermoreHttpPost(randomGeneratedChars, "chal", lastStatus);
}

void loop() {
  maintainEthernet();

  delayedLockButton.loop(digitalRead(BUTTON_PIN) == LOW);

  httpServer.tick(getLockStatus());
  int value = 0;
  if (httpServer.lastCommand[0]) {
    handleCommand(httpServer.lastCommand, REASON_MATTERMORE, &value);
  }

  if (delayedLockButton.get_and_reset_press_count() > 0) {
    Serial.println(LOG_DELAYED_LOCK_PRESSED);
    handleCommand(COMMAND_DELAY, REASON_DELAY_BUTTON, &value);
    delay(100);
  }

#if ENABLE_TURN_ASSIST
  turnAssistLoop();
#endif
  if (millis() - lastPotValuePrintTimestamp > 1000) {
    Serial.println(analogRead(POTENTIOMETER_PIN));
    lastPotValuePrintTimestamp = millis();
  }

  LockStatus newStatus = getLockStatus();
  if (newStatus != inbetween && newStatus != lastStatus) {
    lastStatus = newStatus;
    mattermoreHttpPost(COMMAND_CHANGE, REASON_STATE, lastStatus);
  }
}

void delayedLock() {
  unsigned long start = millis();
  unsigned long next_change = millis() + 500;
  bool play = true;
  toneAC2(BUZZER_1, BUZZER_2, 800);
  while (millis() - start < 10000) {
    if (millis() > next_change) {
      if (play) {
        toneAC2(BUZZER_1, BUZZER_2, 800);
      } else {
        noToneAC2();
      }
      play = !play;
      next_change = millis() + (start + 10000 - millis()) / 20;
    }
  }
  noToneAC2();
  lockDoor();
}

void handleCommand(const char *cmd, const char *why, int *val) {
  // TODO implement challenge/response here and only allow other commands once
  // challenge/response succeeded
  //  this to prevent an attacker from tripping the breaker, thus making the
  //  Arduino (and the replay prevention counter) reset
  // The attacker can then replay an already sent packet they MITM'ed earlier.

  if (strcmp(COMMAND_STATUS, cmd) == 0) {
    *val = getLockStatus();
  } else {
    *val = analogRead(POTENTIOMETER_PIN);
    if (strcmp(COMMAND_OPEN, cmd) == 0) {
      openDoor();
    } else if (strcmp(COMMAND_LOCK, cmd) == 0) {
      lockDoor();
    } else if (strcmp(COMMAND_DELAY, cmd) == 0) {
#if ENABLE_BOMBPLANTSOUND
      // Play "bomb has been planted" on koin if enabled
      mattermoreHttpPost(cmd, REASON_LOCKING, *val);
#endif
      delayedLock();
    } else if (strcmp(COMMAND_CALIBRATE_OPEN_POS, cmd) == 0) {
      EEPROM.put(OPEN_POS_ADDRESS, *val);
    } else if (strcmp(COMMAND_CALIBRATE_OPEN_BND, cmd) == 0) {
      EEPROM.put(OPEN_BOUND_ADDRESS, *val);
    } else if (strcmp(COMMAND_CALIBRATE_CLOSE_POS, cmd) == 0) {
      EEPROM.put(CLOSED_POS_ADDRESS, *val);
    } else if (strcmp(COMMAND_CALIBRATE_CLOSE_BND, cmd) == 0) {
      EEPROM.put(CLOSED_BOUND_ADDRESS, *val);
    } else {
      mattermoreHttpPost(COMMAND_INVALID_COMMAND, REASON_PANIC, getLockStatus());
    }
  }
  mattermoreHttpPost(cmd, why, *val);
}

LockStatus getLockStatus() {
  int open_value, closed_value;
  EEPROM.get(OPEN_POS_ADDRESS, open_value);
  EEPROM.get(CLOSED_POS_ADDRESS, closed_value);
  int open_bound, closed_bound;
  EEPROM.get(OPEN_BOUND_ADDRESS, open_bound);
  EEPROM.get(CLOSED_BOUND_ADDRESS, closed_bound);
  bool toward_open_is_positive = (open_value - closed_value) > 0;

  int current_value = analogRead(POTENTIOMETER_PIN);
  if (toward_open_is_positive == (current_value > open_bound)) {
    return open;
  }
  if (toward_open_is_positive == (current_value < closed_bound)) {
    return closed;
  }
  return inbetween;
}

void lockDoor() {
  if (getLockStatus() == closed)
    return;
  int desired;
  EEPROM.get(CLOSED_POS_ADDRESS, desired);
  bringToState(desired);
}

void openDoor() {
  if (getLockStatus() == open)
    return;
  int desired;
  EEPROM.get(OPEN_POS_ADDRESS, desired);
  bringToState(desired);
}

void bringToState(int desired_value) {
  int open_value, closed_value;
  EEPROM.get(OPEN_POS_ADDRESS, open_value);
  EEPROM.get(CLOSED_POS_ADDRESS, closed_value);
  int current_value = analogRead(POTENTIOMETER_PIN);
  bool initial_to_desired_positive = (desired_value - current_value) >= 0;
  bool toward_open_is_positive = (open_value - closed_value) > 0;
  bool direction = initial_to_desired_positive == toward_open_is_positive;

  uint32_t last_move_check = millis();
  int last_check_value = current_value;
  int least_movement = abs(open_value - closed_value) / 20;

  turnDirection(direction);

  while (true) {
    current_value = analogRead(POTENTIOMETER_PIN);
    if (initial_to_desired_positive == (current_value > desired_value)) {
      turnHalt();
      break;
    }
    if (millis() - last_move_check > 1000) {
      int expectedValue = last_check_value;
      expectedValue += least_movement * (initial_to_desired_positive ? 1 : -1);
      if (initial_to_desired_positive == (expectedValue > current_value)) {
        // Send passed_turning_deadline panic
        mattermoreHttpPost(COMMAND_PASSED_TURNING_DEADLINE, REASON_PANIC,
                           getLockStatus());
        // Move all the way
        turnDirection(direction);
        uint32_t starttime = millis();
        while (millis() - starttime < 5000) {
          if (((millis() - starttime) / 250) % 2 == 0) {
            toneAC2(BUZZER_1, BUZZER_2, 300);
          } else {
            noToneAC2();
          }
        }
        turnHalt();
        mattermoreHttpPost(direction ? COMMAND_OPENED_UNCLEANLY
                                     : COMMAND_CLOSED_UNCLEANLY,
                           REASON_PANIC, getLockStatus());
        break;
      } else {
        last_move_check = millis();
        last_check_value = current_value;
      }
    }
  }
  // Wait for the lock to end up in a stable position
  do {
    last_check_value = current_value;
    delay(500);
    current_value = analogRead(POTENTIOMETER_PIN);
  } while (abs((last_check_value - current_value)) > ANALOG_PRECISION);
#if ENABLE_TURN_ASSIST
  turnAssistLastValue = analogRead(POTENTIOMETER_PIN);
#endif
}

#if ENABLE_TURN_ASSIST
void turnAssistLoop() {
  int current_value = analogRead(POTENTIOMETER_PIN);
  // don't assist after just having turned to prevent humans trying to get their
  // key out activating turn assist again
  if (millis() - turnAssistLastMove < 10000) {
    turnAssistLastValue = current_value;
    return;
  }

  int open_value, closed_value;
  EEPROM.get(OPEN_POS_ADDRESS, open_value);
  EEPROM.get(CLOSED_POS_ADDRESS, closed_value);
  bool toward_open_is_positive = (open_value - closed_value) > 0;

  int turnassist_minimum =
      max(abs(open_value - closed_value) / 20, ANALOG_PRECISION);
  // Serial.print("minimum = ");
  // Serial.println(turnassist_minimum);
  // Serial.println(last_value_turnassist);
  Serial.println(current_value);

  if (toward_open_is_positive == (current_value > open_value)) {
    return; // no need to turnassist, we're after the open value
  }
  if (toward_open_is_positive == (current_value < closed_value)) {
    return; // no need to turnassist, we're before the closed value
  }
  if (abs(current_value - turnAssistLastValue) > turnassist_minimum) {
    Serial.print("Deciding to move based on value = ");
    Serial.print(current_value);
    Serial.print(" compared to ");
    Serial.println(turnAssistLastValue);
    bool going_toward_open = (toward_open_is_positive ==
                              ((current_value - turnAssistLastValue) > 0));
    int value;
    handleCommand(going_toward_open ? "open" : "lock", "turnassist", &value);
    turnAssistLastMove = millis();
  }
}
#endif

/**
 * Servo functions
 * 94 ms is hold pos
 * > 94 -> open, CCW from the back
 * < 94 -> close, CW from the back
 */
void turnOpen() {
  digitalWrite(RELAY_PIN, HIGH);
  door.write(10);
  Serial.println(LOG_TURNING_OPEN);
}
void turnClose() {
  digitalWrite(RELAY_PIN, HIGH);
  door.write(170);
  Serial.println(LOG_TURNING_CLOSE);
}
void turnHalt() {
  digitalWrite(RELAY_PIN, LOW);
  door.write(94);
  Serial.println(LOG_TURNING_HALT);
}

void turnDirection(bool directionIsOpen) {
  if (directionIsOpen) {
    turnOpen();
  } else {
    turnClose();
  }
}
