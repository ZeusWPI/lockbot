#include "http_server.hpp"

#include <Arduino.h>
#include <Ethernet.h>

#include <sha256.h>

#include "ethernet.hpp"
#include "strings.hpp"
#include "tokens.hpp"
#include "util.hpp"

static bool clientParseHeaders(EthernetClient *client, uint8_t *outHmac) {
  if (!client->find((char *)HEADER_PREFIX_HMAC))
    return false;
  char octet[3] = {0};
  for (int i = 0; i < 32; i++) {
    int first = client->read();
    if (first < 0 || first == '\n')
      return false;
    int second = client->read();
    if (second < 0 || second == '\n')
      return false;
    octet[0] = (char)first;
    octet[1] = (char)second;
    *outHmac = strtol(octet, 0, 16);
    outHmac++;
  }

  if (!client->find((char *)"\r\n\r\n"))
    return false;

  return true;
}

static size_t clientReadBody(EthernetClient *client, uint8_t *outBuf,
                             size_t outBufLength) {
  size_t bytesRead = 0;
  while (bytesRead < outBufLength && client->available()) {
    outBuf[bytesRead] = client->read();
    bytesRead++;
  }
  return bytesRead;
}

// Note: bodyBuf has to be zero-terminated
static void parseBody(const uint8_t *bodyBuf, uint64_t *outCommandCounter,
                      char *outCommandBuf, size_t outCommandBufSize) {
  outCommandCounter = 0;
  const char *bodyStr = (char *)bodyBuf;
  while (bodyStr[0] && bodyStr[0] != ';') {
    *outCommandCounter *= 10;
    *outCommandCounter += bodyStr[0] - '0';
    bodyStr++;
  }
  if (bodyStr[0] != ';') {
    return;
  }
  bodyStr++; // skip ';'
  strncpy(outCommandBuf, bodyStr, outCommandBufSize);
}

static void clientSend400(EthernetClient *client, const char *message) {
  client->println(HTTP_400);
  client->print(HEADER_PREFIX_CONTENT_LENGTH);
  client->println(strlen(message));
  client->println();
  client->print(message);
}

HttpServer::HttpServer(uint16_t port) : server(port), currentCommandCounter(), lastCommand() {}

void HttpServer::start() { server.begin(); }

void HttpServer::tick(LockStatus lockStatus) {
  memset(lastCommand, 0, sizeof(lastCommand));

  EthernetClient client = server.available();
  ClientCleanupGuard clientCleanupGuard(client);
  if (!client)
    return;

  uint8_t hmacReceived[32]{};
  if (!clientParseHeaders(&client, hmacReceived)) {
    clientSend400(&client, COMMAND_NO_HMAC);
    // Not enough memory :/
    // mattermoreHttpPost(COMMAND_NO_HMAC, REASON_ATTACK, 0);
    return;
  }

  // allow reading body of up to 128 bytes (should be enough)
  uint8_t bodyBuf[128 + 1]{};
  size_t bodyLength = clientReadBody(&client, bodyBuf, sizeof(bodyBuf) - 1);
  if (client.available()) {
    clientSend400(&client, COMMAND_TOO_LONG);
    // Not enough memory :/
    // mattermoreHttpPost(COMMAND_TOO_LONG, REASON_ATTACK, 0);
    return;
  }
  // Serial.println((char *)bodyBuf);

  Sha256Class sha256{};
  sha256.initHmac(DOWN_COMMAND_KEY, strlen((const char *)DOWN_COMMAND_KEY));
  sha256.write(bodyBuf, bodyLength);
  uint8_t *hmacCalculated = sha256.resultHmac();
  if (memcmp(hmacCalculated, hmacReceived, 32) != 0) {
    clientSend400(&client, COMMAND_WRONG_HMAC);
    // Not enough memory :/
    // mattermoreHttpPost(COMMAND_WRONG_HMAC, REASON_ATTACK, 0);
    return;
  }

  uint64_t receivedCommandCounter{};
  parseBody(bodyBuf, &receivedCommandCounter, lastCommand, sizeof(lastCommand));

  if (receivedCommandCounter <= currentCommandCounter) {
    clientSend400(&client, COMMAND_REPLAY);
    // Not enough memory :/
    // mattermoreHttpPost(COMMAND_REPLAY, REASON_ATTACK, 0);
    return;
  }
  currentCommandCounter = receivedCommandCounter;

  client.println(HTTP_200);
  client.print(HEADER_PREFIX_CONTENT_LENGTH);
  client.println(1);
  client.println();
  client.print((int)lockStatus);
}
