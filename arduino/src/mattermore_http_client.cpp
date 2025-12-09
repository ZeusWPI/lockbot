#include "mattermore_http_client.hpp"

#include <Arduino.h>
#include <Ethernet.h>

#include <sha256.h>

#include "ethernet.hpp"
#include "strings.hpp"
#include "tokens.hpp"

/**
 * Makes a http POST request to mattermore on `/doorkeeper`, with `cmd`, `why`,
 * and `val` as body parameters.
 */
bool mattermoreHttpPost(const char *cmd, FlashString why, int val) {
  EthernetClient client;
  ClientCleanupGuard clientCleanupGuard(client);

  Serial.println(FSTR_LOG_MATTERMORE_CONNECTING);
  if (client.connect(MATTERMORE_SERVER_HOST, MATTERMORE_SERVER_PORT)) {
    char bodyBuf[64];
    size_t bodyLength = snprintf(bodyBuf, sizeof(bodyBuf),
                                 FSTR_FMT_MATTERMORE_BODY, cmd, why, val);
    Serial.print(FSTR_LOG_MATTERMORE_SENDING);
    Serial.println(bodyBuf);

    Sha256Class sha256;
    sha256.initHmac(UP_COMMAND_KEY, strlen((const char *)UP_COMMAND_KEY));
    sha256.write(bodyBuf, bodyLength);
    uint8_t *hmacCalculated = sha256.resultHmac();

    char hmacHexString[32 * 2 + 1]{};
    {
      char *ptr = hmacHexString;
      for (int i = 0; i < 32; i++) {
        snprintf(ptr, 3, FSTR_FMT_HEX, hmacCalculated[i]);
        ptr += 2;
      }
    }

    client.println(FSTR_HTTP_POST_DOORKEEPER);

    // Headers
    {
      client.print(FSTR_HEADER_PREFIX_HOST);
      client.println(MATTERMORE_SERVER_HOST);

      client.print(STR_HEADER_PREFIX_HMAC);
      client.println(hmacHexString);

      client.print(FSTR_HEADER_PREFIX_CONTENT_LENGTH);
      client.println(bodyLength);

      client.println(FSTR_HEADER_CONNECTION_CLOSE);

      client.println();
    }

    // Body
    client.println(bodyBuf);

    return true;
  }
  Serial.println(FSTR_LOG_MATTERMORE_CONNECTING_FAILED);
  return false;
}
