// Link layer protocol implementation
#include "llopen_connection.h"
#include "llclose_connection.h"
#include "llread_frame.h"
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
int llread(unsigned char *packet) { return llread_frame(packet); }

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
  return llclose_connection(connectionParams);
}
