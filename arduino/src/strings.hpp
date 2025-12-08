#pragma once

#include <Arduino.h>

#ifdef STRINGS_CPP
#define STR(name, value) extern const char name[] = value
#else
#define STR(name, value) extern const char name[]
#endif

STR(COMMAND_STATUS, "status");
STR(COMMAND_OPEN, "open");
STR(COMMAND_LOCK, "lock");
STR(COMMAND_DELAY, "delay");
STR(COMMAND_CALIBRATE_OPEN_POS, "calibrate_open_pos");
STR(COMMAND_CALIBRATE_OPEN_BND, "calibrate_open_bnd");
STR(COMMAND_CALIBRATE_CLOSE_POS, "calibrate_close_pos");
STR(COMMAND_CALIBRATE_CLOSE_BND, "calibrate_close_bnd");

STR(COMMAND_UP, "up");
STR(COMMAND_CHANGE, "change");

STR(COMMAND_NO_HMAC, "no_hmac");
STR(COMMAND_TOO_LONG, "too_long");
STR(COMMAND_WRONG_HMAC, "wrong_hmac");
STR(COMMAND_REPLAY, "replay");

STR(REASON_BOOT, "boot");
STR(REASON_CHALLENGE, "chal");
STR(REASON_STATE, "state");
STR(REASON_DELAY_BUTTON, "delaybutton");
STR(REASON_MATTERMORE, "mattermore");
STR(REASON_LOCKING, "locking");
STR(REASON_ATTACK, "attack");
STR(REASON_PANIC, "panic");

STR(HTTP_POST_DOORKEEPER, "POST /doorkeeper HTTP/1.1");
STR(HTTP_200, "HTTP/1.1 200");
STR(HTTP_400, "HTTP/1.1 400");
STR(HEADER_CONNECTION_CLOSE, "Connection: close");
STR(HEADER_PREFIX_CONTENT_LENGTH, "Content-Length: ");
STR(HEADER_PREFIX_HMAC, "HMAC: ");
STR(HEADER_PREFIX_HOST, "Host: ");

STR(LOG_BOOTING, "Booting up...");
STR(LOG_INITIALIZING_SERVO, "Initialising servo controller...");
STR(LOG_INITIALIZING_ETHERNET, "Initialising ethernet...");
STR(LOG_STARTING_WEBSERVER, "Starting webserver...");
STR(LOG_DELAYED_LOCK_PRESSED, "Delayed lock button pressed");
STR(LOG_TURNING_OPEN, "Turning towards open");
STR(LOG_TURNING_CLOSE, "Turning towards close");
STR(LOG_TURNING_HALT, "Turning halt");
STR(LOG_MATTERMORE_CONNECTING, "Connecting to mattermore...");
STR(LOG_MATTERMORE_CONNECTING_FAILED, "Failed to connect to mattermore");
STR(LOG_MATTERMORE_SENDING, "Sending: ");

STR(PANIC_INVALID_COMMAND, "Invalid command");
STR(PANIC_PASSED_TURNING_DEADLINE, "Passed turning deadline");

#undef STR
