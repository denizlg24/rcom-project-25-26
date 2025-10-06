// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "serial_port.h"
#include <stdio.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {
  LinkLayer options = {0};

  strcpy(options.serialPort, serialPort);

  options.role = strcmp("tx", role) == 0 ? LlTx : LlRx;

  options.baudRate = baudRate;
  options.nRetransmissions = nTries;
  options.timeout = timeout;

  llopen(options);
}
