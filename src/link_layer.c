// Link layer protocol implementation
#include "llopen_connection.h"
#include "llwrite_frame.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

static LinkLayer connectionParams;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
  connectionParams = connectionParameters;
  if (openSerialPort(connectionParameters.serialPort,
                     connectionParameters.baudRate) < 0) {
    perror("openSerialPort");
    exit(EXIT_FAILURE);
  }

  printf("Serial port %s opened\n", connectionParameters.serialPort);

  if (connectionParameters.role == LlTx)
    return llopen_tx_connection(connectionParameters);
  else
    return llopen_rx_connection(connectionParameters);
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
  return llwrite_frame(buf, bufSize, connectionParams);
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
  // TODO: Implement this function`
  unsigned char byte;
  while (1) {
    int bytes = readByteSerialPort(&byte);
    if (bytes > 0) {
      printf("var = 0x%02X\n", byte);
    }
  }

  return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
  // TODO: Implement this function

  return 0;
}
