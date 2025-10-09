#include "frame_helpers.h"
#include "link_layer_state_machine.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>

unsigned char compute_bcc2(const unsigned char *data, size_t length) {
  unsigned char bcc2 = 0;
  for (size_t i = 0; i < length; i++) {
    bcc2 ^= data[i];
  }
  return bcc2;
}

int send_frame(const unsigned char *frame, size_t size) {
  if (writeBytesSerialPort(frame, size) < 0) {
    perror("writeBytesSerialPort");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
    }
    exit(EXIT_FAILURE);
  }
  return 0;
}

int read_frame(const unsigned char *expected, unsigned char *abortSignal) {
  unsigned char byte;
  State currState = START;
  (*abortSignal) = 0;
  while (currState != STOP && (*abortSignal) != 1) {
    int bytes = readByteSerialPort(&byte);
    if (bytes == -1) {
      perror("readByteSerialPort");
      if (closeSerialPort() < 0) {
        perror("closeSerialPort");
      }
      exit(EXIT_FAILURE);
    }

    if (bytes > 0 &&
        !processStateMachine(&currState, byte, abortSignal, expected)) {
      break;
    }
  }

  return currState == STOP;
}