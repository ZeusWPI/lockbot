#pragma once

#include <Arduino.h>
#include <Ethernet.h>

void setupEthernet();
void maintainEthernet();

class ClientCleanupGuard {
private:
  EthernetClient &m_client;

public:
  ClientCleanupGuard(EthernetClient &client);
  ~ClientCleanupGuard();
};
