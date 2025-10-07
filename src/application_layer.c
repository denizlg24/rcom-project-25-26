// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {
  LinkLayer options = {0};

  strcpy(options.serialPort, serialPort);

  options.role = strcmp("tx", role) == 0 ? LlTx : LlRx;

  options.baudRate = baudRate;
  options.nRetransmissions = nTries;
  options.timeout = timeout;

  if (llopen(options) < 0) {
    exit(-1);
  }

  if (options.role == LlTx) {
    unsigned char buf1[] = "Hello";
    printf("=== Test 1: short text ===\n");
    llwrite(buf1, sizeof(buf1) - 1);
  } else {
    unsigned char *packet = (unsigned char *)malloc(MAX_PAYLOAD_SIZE);
    llread(packet);
  }
}
