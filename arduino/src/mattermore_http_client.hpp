#pragma once

#include <Arduino.h>

#include "strings.hpp"

bool mattermoreHttpPost(const char *cmd, FlashString why, int val);
