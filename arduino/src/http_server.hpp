#pragma once

#include <Arduino.h>
#include <Ethernet.h>

#include "util.hpp"

class HttpServer {
private:
  EthernetServer server;
  uint64_t currentCommandCounter;

public:
  char lastCommand[32];

public:
  HttpServer(uint16_t port);
  void start();
  void tick(LockStatus lockStatus);
};
