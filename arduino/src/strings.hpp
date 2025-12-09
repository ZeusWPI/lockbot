#pragma once

#include <Arduino.h>

#define FlashString const __FlashStringHelper *

#ifdef STRINGS_CPP
#define STR(name, value) extern const char STR_##name[] = value;
#define FSTR(name, value) extern const char FSTR_##name[] PROGMEM = value;
#else
#define STR(name, value) extern const char STR_##name[];
#define FSTR(name, value) extern const __FlashStringHelper FSTR_##name[];
#endif

FSTR(FMT_MATTERMORE_BODY, "cmd=%s&why=%S&val=%d")
FSTR(FMT_HEX, "%.2X")

FSTR(COMMAND_STATUS, "status")
FSTR(COMMAND_OPEN, "open")
FSTR(COMMAND_LOCK, "lock")
STR(COMMAND_DELAY, "delay")
FSTR(COMMAND_CALIBRATE_OPEN_POS, "calibrate_open_pos")
FSTR(COMMAND_CALIBRATE_OPEN_BND, "calibrate_open_bnd")
FSTR(COMMAND_CALIBRATE_CLOSE_POS, "calibrate_close_pos")
FSTR(COMMAND_CALIBRATE_CLOSE_BND, "calibrate_close_bnd")

STR(COMMAND_UP, "up")
STR(COMMAND_CHANGE, "change")

STR(COMMAND_NO_HMAC, "no_hmac")
STR(COMMAND_TOO_LONG, "too_long")
STR(COMMAND_WRONG_HMAC, "wrong_hmac")
STR(COMMAND_REPLAY, "replay")

STR(COMMAND_INVALID_COMMAND, "invalid_command")
STR(COMMAND_PASSED_TURNING_DEADLINE, "passed_turning_deadline")
STR(COMMAND_CLOSED_UNCLEANLY, "closed_uncleanly")
STR(COMMAND_OPENED_UNCLEANLY, "opened_uncleanly")

FSTR(REASON_BOOT, "boot")
FSTR(REASON_CHALLENGE, "chal")
FSTR(REASON_STATE, "state")
FSTR(REASON_DELAY_BUTTON, "delaybutton")
FSTR(REASON_MATTERMORE, "mattermore")
FSTR(REASON_LOCKING, "locking")
FSTR(REASON_ATTACK, "attack")
FSTR(REASON_PANIC, "panic")

FSTR(HTTP_POST_DOORKEEPER, "POST /doorkeeper HTTP/1.1")
FSTR(HTTP_200, "HTTP/1.1 200")
FSTR(HTTP_400, "HTTP/1.1 400")
FSTR(HEADER_CONNECTION_CLOSE, "Connection: close")
FSTR(HEADER_PREFIX_CONTENT_LENGTH, "Content-Length: ")
STR(HEADER_PREFIX_HMAC, "HMAC: ")
FSTR(HEADER_PREFIX_HOST, "Host: ")
STR(END_OF_HEADERS, "\r\n\r\n")

FSTR(LOG_BOOTING, "Booting up...")
FSTR(LOG_INITIALIZING_SERVO, "Initialising servo controller...")
FSTR(LOG_INITIALIZING_ETHERNET, "Initialising ethernet...")
FSTR(LOG_STARTING_WEBSERVER, "Starting webserver...")
FSTR(LOG_DELAYED_LOCK_PRESSED, "Delayed lock button pressed")
FSTR(LOG_TURNING_OPEN, "Turning towards open")
FSTR(LOG_TURNING_CLOSE, "Turning towards close")
FSTR(LOG_TURNING_HALT, "Turning halt")
FSTR(LOG_MATTERMORE_CONNECTING, "Connecting to mattermore...")
FSTR(LOG_MATTERMORE_CONNECTING_FAILED, "Failed to connect to mattermore")
FSTR(LOG_MATTERMORE_SENDING, "Sending: ")

#undef STR

size_t strlen(FlashString s);
int strcmp(const char *s1, FlashString s2);
int snprintf(char *s, size_t n, FlashString fmt, ...);
