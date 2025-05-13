// Compile for Arduino Uno

#define ARDUINOJSON_USE_LONG_LONG 1
#include <PWMServo.h>    // install from Arduino libraries manager (version 2.1.0)
#include <Ethernet.h>    // install from Arduino libraries manager (version 2.0.2)
#include "./toneAC2/toneAC2.h"
#include "./toneAC2/toneAC2.cpp"
#include <SPI.h>         // built-in
#include <EEPROM.h>      // built-in
#include "./tokens.h"
#include "./sha256/sha256.h"
#include "./sha256/sha256.cpp"
#include "./util.h"


#define OPEN_POS_ADDRESS 0
#define CLOSED_POS_ADDRESS 8
#define OPEN_BOUND_ADDRESS 12
#define CLOSED_BOUND_ADDRESS 16

#define ENABLE_BOMBPLANTSOUND true

// Connect wiper pin of 10k potentiometer to A0, the other pins to VCC and ground
#define POTENTIOMETER_PIN A0

// Connect IN pin of relay to pin 3, VCC pin to the motor power supply 5V, ground to ground
// common to the motor power supply 5V cable, NO (normal open) to the 5V cable of the motor
#define RELAY_PIN 3

// Connect one leg of the button to 5, the other to ground
#define BUTTON_PIN 5

// Leave this pin unconnected (or alternatively, connect a short length of wire to the pin and leave unconnected)
// This is used to collect randomness via ADC noise
#define UNCONNECTED_RANDOM_PIN A1

// SERVO_PIN_A = 9
// Connect the data pin of the servo motor to pin 9, ground to ground. VCC should already be connected to NO of the relay

// toneAC buzzer pins
#define BUZZER_1 4
#define BUZZER_2 7

#define ANALOG_PRECISION 4

EthernetServer server(80);
EthernetClient client;

PWMServo door;

Debounced delayedLockButton;

int last_value_turnassist;
uint32_t last_turnassist_move = 0;

char* hmac_header_name = "HMAC: ";

char generated_random[32+1] = {};
bool received_challenge_response = false;


bool getHMACHeader(EthernetClient* client, uint8_t* buf) {
  if (!client->find(hmac_header_name)) return false;

  char octet[3] = {0};
  for (int i = 0; i < 32; i++) {
    int first = client->read();
    if (first < 0 || first == '\n') return false;
    int second = client->read();
    if (second < 0 || second == '\n') return false;
    octet[0] = (char) first;
    octet[1] = (char) second;
    *buf = strtol(octet, 0, 16);
    buf++;
  }
  return true;
}

void give400(EthernetClient* client, const char* message) {
  client->println(F("HTTP/1.1 400"));
  client->println(F("Connection: close"));
  client->println();
  client->print(message);
  client->flush();
  client->stop();
}

uint64_t current_command_counter = 0;

// Process incoming HTTP request
bool handleIncoming(String *command)
{
  client = server.available();
  if (!client) return false;
  uint8_t hmac_message[32] = {0};
  if (!getHMACHeader(&client, hmac_message)) {
    give400(&client, "no_hmac");
    // Not enough memory :/
    // sendMattermoreData("no_hmac", "attack", 0);
    return false;
  }
  // skip rest of headers
  client.find("\r\n\r\n");
  int bytes_read = 0;
  uint8_t body_buffer[128+1] = {0};
  // allow reading body of up to 128 bytes (should be enough)
  while (bytes_read < 128 && client.available()) {
    body_buffer[bytes_read] = client.read();
    bytes_read++;
  }

  if (client.available()) {
    give400(&client, "LENGTH");
    // Not enough memory :/
    // sendMattermoreData("too_long", "attack", 0);
    return false;
  }
  Sha256Class hmac_generator;
  hmac_generator.initHmac(DOWN_COMMAND_KEY, strlen((const char*) DOWN_COMMAND_KEY));
  Serial.println((char*) body_buffer);
  hmac_generator.write(body_buffer, bytes_read);
  uint8_t* hmac_calculated = hmac_generator.resultHmac();
  if (memcmp(hmac_calculated, hmac_message, 32) != 0) {
    give400(&client, "wrong_hmac");
    // Not enough memory :/
    // sendMattermoreData("wrong_hmac", "attack", 0);
    return false;
  }

  // parse body
  uint64_t received_ctr = 0;
  const char* body_str = body_buffer;
  while (*body_str != ';' && *body_str != 0) {
    received_ctr *= 10;
    received_ctr += *body_str - '0';
    body_str++;
  }
  if (received_ctr <= current_command_counter) {
    give400(&client, "REPLAY");
    // Not enough memory :/
    // sendMattermoreData("replay", "attack", 0);
    return false;
  }
  current_command_counter = received_ctr;
  body_str++; // skip ';'
  *command += body_str;

  client.println("HTTP/1.1 200");
  client.println("Content-Lenght: 1");
  client.println();
  client.print((int) getLockStatus());
  client.flush();
  client.stop();
  return true;
}

// Sends the processed commands back to mattermore to send to the channel
bool sendMattermoreData(const char* command, const char* reason, int value)
{
  EthernetClient requestclient;
  Serial.print("Sending to mattermore: ");
  Serial.println(reason);
  if (requestclient.connect(MATTERMORE_SERVER_HOST, MATTERMORE_SERVER_PORT))
  {
    String msg = String("cmd="+String(command)+"&why="+String(reason)+"&val="+String(value));
    Serial.println(msg);
    Sha256Class hmac_generator;
    hmac_generator.initHmac(UP_COMMAND_KEY, strlen((const char*) UP_COMMAND_KEY));
    hmac_generator.write(msg.c_str(), msg.length());
    uint8_t* hmac_calculated = hmac_generator.resultHmac();
    char hmac_header_hex[32*2+1] = {0};
    char* hmac_header_build = hmac_header_hex;
    for (int i = 0; i < 32; i++) {
      sprintf(hmac_header_build, "%.2X", hmac_calculated[i]);
      hmac_header_build += 2;
    }


    requestclient.println(F("POST /doorkeeper HTTP/1.1"));
    requestclient.print(F("Host: "));
    requestclient.println(MATTERMORE_SERVER_HOST);
    requestclient.print(hmac_header_name);
    requestclient.println(hmac_header_hex);
    requestclient.print(F("Content-Length: "));
    requestclient.println(msg.length());
    requestclient.println(F("Connection: close"));
    requestclient.println();
    requestclient.println(msg);
    requestclient.flush();
    requestclient.stop();
    return true;
  }
  Serial.println(F("Connecting failed"));
  requestclient.stop();
  return false;
}

// Maintain ethernet connection
void maintainEthernet()
{
  switch (Ethernet.maintain())
  {
    case 1:
    // Serial.println("Error: renewed fail");
    break;
    case 2:
    // Serial.println("Renewed success");
    break;
    case 3:
    // Serial.println("Error: rebind fail");
    break;
    case 4:
    // Serial.println("Rebind success");
    break;
    default:
    break;
  }
}

/*
Servo functions
94 ms is hold pos
> 94 -> open, CCW from the back
< 94 -> close, CW from the back
*/
void turnOpen()   {digitalWrite(RELAY_PIN, HIGH); door.write(10); Serial.println(F("Turning towards open"));}
void turnClose()  {digitalWrite(RELAY_PIN, HIGH); door.write(170); Serial.println(F("Turning towards close"));}
void turnHalt()   {digitalWrite(RELAY_PIN, LOW); door.write(94); Serial.println(F("Turning halt"));}

void turnDirection(bool directionIsOpen) {
  if (directionIsOpen) {
    turnOpen();
  } else {
    turnClose();
  }
}

#define MS_DIT 50
#define FREQ 600

void panic(const char* msg) {
  turnClose();

  uint32_t starttime = millis();
  while (millis() - starttime < 5000) {
    if (((millis() - starttime) / 250) % 2 == 0) {
      toneAC2(BUZZER_1, BUZZER_2, 300);
    } else {
      noToneAC2();
    }
  }

  turnHalt();
  sendMattermoreData(msg, "panic", getLockStatus());
  const char morse[] = ".--. .- -. .. -.-.";
  int index = 0;
  starttime = millis();
  bool interelement = false;
  while (true) {
    uint32_t time_passed = millis() - starttime;
    char current = morse[index];
    uint32_t duration;
    if (interelement) {
      duration = MS_DIT;
      noToneAC2();
    } else if (current == '.') {
      duration = MS_DIT;
      toneAC2(BUZZER_1, BUZZER_2, 800);
    } else if (current == '-') {
      duration = 3 * MS_DIT;
      toneAC2(BUZZER_1, BUZZER_2, 800);
    } else if (current == ' ') {
      duration = 2 * MS_DIT; // 3 - 1 because we already did interelement
      noToneAC2();
    } else if (current == 0) {
      duration = 5 * MS_DIT; // 7 - 2 because we already did interelement and will do interelement again
      noToneAC2();
    } else {
      duration = 10 * MS_DIT;
      toneAC2(BUZZER_1, BUZZER_2, 800);
    }
    if (time_passed > duration) {
      starttime = millis();
      if (interelement) {
        if (current == 0) {
          Serial.println(msg);
        }
        index = (index + 1) % (sizeof(morse));
      }
      interelement = !interelement;
    }
  }
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
      if (initial_to_desired_positive == (last_check_value + (initial_to_desired_positive ? 1 : -1) * least_movement > current_value)) {
        panic("Passed turning deadline"); // this will never return
        return;
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
  last_value_turnassist = analogRead(POTENTIOMETER_PIN);
}

void lockDoor()
{
  if (getLockStatus() == closed) return;
  int desired;
  EEPROM.get(CLOSED_POS_ADDRESS, desired);
  bringToState(desired);
}

void openDoor()
{
  if (getLockStatus() == open) return;
  int desired;
  EEPROM.get(OPEN_POS_ADDRESS, desired);
  bringToState(desired);
}

lock_status getLockStatus()
{
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

void handleCommand(const char* command, const char* reason, int* value)
{
  int current_value = analogRead(POTENTIOMETER_PIN);
  *value = current_value;
  // TODO implement challenge/response here and only allow other commands once challenge/response succeeded
  //  this to prevent an attacker from tripping the breaker, thus making the Arduino (and the replay prevention counter) reset
  // The attacker can then replay an already sent packet they MITM'ed earlier.

  if (strcmp("open", command) == 0) {
    openDoor();
  } else if (strcmp("lock", command) == 0) {
    lockDoor();
  } else if (strcmp("delay", command) == 0) {
    if(ENABLE_BOMBPLANTSOUND) sendMattermoreData(command, "locking", *value); //play "bomb has been planted" if enabled
    delayedLock();
  } else if (strcmp("status", command) == 0) {
    *value = getLockStatus();
  } else if (strcmp("calibrate_open_pos", command) == 0) {
    EEPROM.put(OPEN_POS_ADDRESS, current_value);
  } else if (strcmp("calibrate_open_bnd", command) == 0) {
    EEPROM.put(OPEN_BOUND_ADDRESS, current_value);
  } else if (strcmp("calibrate_close_pos", command) == 0) {
    EEPROM.put(CLOSED_POS_ADDRESS, current_value);
  } else if (strcmp("calibrate_close_bnd", command) == 0) {
    EEPROM.put(CLOSED_BOUND_ADDRESS, current_value);
  } else {
    panic("Invalid command");
  }
  sendMattermoreData(command, reason, *value);
}

lock_status last_status;

void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  toneAC2(BUZZER_1, BUZZER_2, 500);
  last_value_turnassist = analogRead(POTENTIOMETER_PIN);
  Serial.begin(9600);
  Serial.println("Booting up ...");

  Serial.println("Initialising servo controller ...");
  // Config servo motor control pin, 500-2500 is for the 10kg/cm servo
  // SERVO_PIN_A = pin 9 see http://arduiniana.org/libraries/pwmservo/
  door.attach(SERVO_PIN_A);
  turnHalt();

  Serial.println("Initialising ethernet ...");

  byte mac[] = {0x00, 0x20, 0x91, 0x10, 0x00, 0x00};
  IPAddress ip(192, 168, 0, 5);
  IPAddress subnetMask(255, 255, 252, 0);
  IPAddress gateway(192, 168, 0, 1);
  Ethernet.begin(mac, ip, gateway, gateway, subnetMask);

  Serial.println("Starting webserver ...");
  server.begin();

  Serial.println("Doorkeeper: online");
  sendMattermoreData("up", "boot", getLockStatus());
  for (int i = 0; i < 32; i++) {
    generated_random[i] = 'A' + (analogRead(UNCONNECTED_RANDOM_PIN) % 26);
  }
  sendMattermoreData((char*) generated_random, "chal", getLockStatus());
  noToneAC2();
  last_status = getLockStatus();
}

void turnAssistLoop() {
  int current_value = analogRead(POTENTIOMETER_PIN);
  // don't assist after just having turned to prevent humans trying to get their key out activating turn assist again
  if (millis() - last_turnassist_move < 10000) {
    last_value_turnassist = current_value;
    return;
  }

  int open_value, closed_value;
  EEPROM.get(OPEN_POS_ADDRESS, open_value);
  EEPROM.get(CLOSED_POS_ADDRESS, closed_value);
  bool toward_open_is_positive = (open_value - closed_value) > 0;

  int turnassist_minimum = max(abs(open_value - closed_value) / 20, ANALOG_PRECISION);
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
  if (abs(current_value - last_value_turnassist) > turnassist_minimum) {
    Serial.print("Deciding to move based on value = ");
    Serial.print(current_value);
    Serial.print(" compared to ");
    Serial.println(last_value_turnassist);
    bool going_toward_open = (toward_open_is_positive == ((current_value - last_value_turnassist) > 0));
    int value;
    handleCommand(going_toward_open ? "open" : "lock", "turnassist", &value);
    last_turnassist_move = millis();
  }
}
uint32_t lastprint = 0;
uint32_t last_change_state = 0;

void loop()
{
  maintainEthernet();
  delayedLockButton.loop(digitalRead(BUTTON_PIN) == LOW);

  String rxCommand;
  int value = 0;
  if (handleIncoming(&rxCommand)) {
    handleCommand(rxCommand.c_str(), "mattermore", &value);
  }

  if (delayedLockButton.get_and_reset_press_count() > 0) {
    Serial.println("delayedClose");
    handleCommand("delay", "delaybutton", &value);
    delay(100);
  }
  // disabled until we get a 10k potentiometer
  // turnAssistLoop();
  if (millis() - lastprint > 1000) {
    Serial.println(analogRead(POTENTIOMETER_PIN));
    lastprint = millis();
  }

  lock_status new_status = getLockStatus();
  if (new_status != inbetween && new_status != last_status) {
    last_status = new_status;
    sendMattermoreData("change", "state", last_status);
  }
}
