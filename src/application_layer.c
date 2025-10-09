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
    unsigned char buf1[] = "Hello world from the transmitter.";
    printf("=== Test 1: short text ===\n");
    llwrite(buf1, sizeof(buf1) - 1);
    unsigned char buf2[] = "This"
                           "\x7E"
                           "is"
                           "\x7D"
                           "another"
                           "\x7E"
                           "test"
                           "\x7D"
                           "string!";
    printf("=== Test 2: short text with stuffing ===\n");
    llwrite(buf2, sizeof(buf2) - 1);
  } else {
    unsigned char *packet = (unsigned char *)malloc(MAX_PAYLOAD_SIZE);
    int read_size = 0;
    while (1) {
      while ((read_size = llread(packet)) < 0);
      if (read_size == 0)
        break;
      printf("[RX] Read packet of size %d bytes.\n", read_size);
      printf("[RX] Packet (ASCII): ");
      for (int i = 0; i < read_size; i++) {
        if (packet[i] >= 32 && packet[i] <= 126)
          printf("%c", packet[i]);
        else
          printf(".");
      }
      printf("\n");
    }
  }
}
