#include "ethernet.hpp"

#include <Arduino.h>
#include <Ethernet.h>

void setupEthernet() {
  byte mac[] = {0x00, 0x20, 0x91, 0x10, 0x00, 0x00};
  IPAddress ip(192, 168, 0, 5);
  IPAddress subnetMask(255, 255, 252, 0);
  IPAddress gateway(192, 168, 0, 1);
  Ethernet.begin(mac, ip, /*dns:*/ gateway, gateway, subnetMask);
}

// Maintain ethernet link
void maintainEthernet() {
  switch (Ethernet.maintain()) {
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

ClientCleanupGuard::ClientCleanupGuard(EthernetClient &client)
    : m_client(client) {}

ClientCleanupGuard::~ClientCleanupGuard() {
  if (m_client) {
    m_client.flush();
    m_client.stop();
  }
}
