#include "frame_helpers.h"
#include "link_layer.h"
#include "link_layer_state_machine.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int readyFor = 0;

int read_header(const unsigned char *expectedHeader) {
  State currState = START;
  unsigned char dummyAbort = 0;
  unsigned char byte;

  while (currState != BCC_OK) {
    int bytes = readByteSerialPort(&byte);
    if (bytes == -1) {
      perror("readByteSerialPort");
      if (closeSerialPort() < 0) {
        perror("closeSerialPort");
      }
      exit(EXIT_FAILURE);
    }

    if (bytes > 0 &&
        !processStateMachine(&currState, byte, &dummyAbort, expectedHeader)) {
      break;
    }
  }
  return 0;
}

int read_packet(unsigned char *packet, const unsigned char *expectedHeader) {

  int headerRead = read_header(expectedHeader);
  if (headerRead != 0) {
    return -1;
  }

  unsigned char byte;
  unsigned char frame[MAX_PAYLOAD_SIZE + 6];
  int i = 4;
  unsigned int flagRead = 0;
  unsigned int escRead = 0;
  memcpy(frame, expectedHeader, 4);
  while (flagRead == 0) {
    int bytes = readByteSerialPort(&byte);
    if (bytes == -1) {
      perror("readByteSerialPort");
      if (closeSerialPort() < 0) {
        perror("closeSerialPort");
      }
      exit(EXIT_FAILURE);
    }
    if (bytes > 0) {
      if (escRead) {
        escRead = 0;
        if (byte == 0x5E) {
          frame[i++] = FLAG;
        } else if (byte == 0x5D) {
          frame[i++] = ESC;
        } else {
          frame[i++] = byte;
        }
        continue;
      }
      if (byte == ESC) {
        escRead = 1;
        continue;
      }
      frame[i] = byte;
      if (byte == FLAG) {
        flagRead = 1;
        unsigned char bcc2 = frame[i - 1];
        int payloadSize = i - 5;

        unsigned char computed = compute_bcc2(frame + 4, payloadSize);

        if (bcc2 != computed) {
          printf("[RX] BCC2 error: expected %02X, got %02X\n", computed, bcc2);
          send_frame(readyFor == 0 ? REJ0_FRAME : REJ1_FRAME, 5);
          return -1;
        }
        readyFor = (readyFor + 1) % 2;
        memcpy(packet, frame + 4, payloadSize);
        send_frame(readyFor == 1 ? RR1_FRAME : RR0_FRAME, 5);
        return payloadSize;
      }
      i++;
      if (i >= MAX_PAYLOAD_SIZE + 6) {
        printf("[RX] Frame too large.\n");
        return -1;
      }
    }
  }
  return -1;
}

int llread_frame(unsigned char *packet) {
  switch (readyFor) {
  case 1:
    return read_packet(packet, I1_HEADER);
  case 0:
  default:
    return read_packet(packet, I0_HEADER);
  }
}