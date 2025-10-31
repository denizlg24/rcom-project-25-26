#include "frame_helpers.h"
#include "link_layer.h"
#include "link_layer_state_machine.h"
#include "link_layer_stats.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int readyFor = 0;

int read_header(const unsigned char *expectedHeader) {
  State currState = START;
  State prevState = START;
  State setState = START;
  unsigned char dummyAbort = 0;
  unsigned char byte;
  int error_frames = 0;
  while (currState != BCC_OK) {
    int bytes = readByteSerialPort(&byte);
    if (bytes == -1) {
      perror("readByteSerialPort");
      if (closeSerialPort() < 0) {
        perror("closeSerialPort");
      }
      exit(EXIT_FAILURE);
    }

    if (bytes > 0) {
      /*int setReceived = !*/ processStateMachine(&setState, byte, &dummyAbort,
                                                  SET_FRAME, NULL);
      int expectedSuccess = !processStateMachine(&currState, byte, &dummyAbort,
                                                 expectedHeader, &error_frames);
      /*int previousReceived = !*/ processStateMachine(
          &prevState, byte, &dummyAbort, readyFor == 0 ? I1_HEADER : I0_HEADER,
          NULL);
      if (expectedSuccess)
        break;
      if (prevState == BCC_OK) {
        ll_stats.error_frames += error_frames;
        printf("[RX] Duplicate frame detected. Resending RR%d.\n", readyFor);
        send_frame(readyFor == 0 ? RR0_FRAME : RR1_FRAME, 5);
        return -1;
      }
      if (setState == STOP) {
        ll_stats.error_frames += error_frames;
        printf("[RX] Got SET Frame again -- Connection was lost -- Sent UA "
               "Again.\n");
        send_frame(UA_FRAME, 5);
        return -2;
      }
    }
  }
  ll_stats.error_frames += error_frames;
  return 0;
}

int read_packet(unsigned char *packet, const unsigned char *expectedHeader) {

  int headerRead = read_header(expectedHeader);
  if (headerRead != 0) {
    return headerRead;
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
          ll_stats.error_frames++;
          send_frame(readyFor == 0 ? REJ0_FRAME : REJ1_FRAME, 5);
          return -1;
        }
        if (rand() % 100 <= BCC1_ERROR_PROB - 1) {
          printf("[RX] BCC1 Simulated error\n");
          ll_stats.error_frames++;
          return -1;
        }
        if (rand() % 100 <= BCC2_ERROR_PROB - 1) {
          printf("[RX] BCC2 Simulated error\n");
          ll_stats.error_frames++;
          send_frame(readyFor == 0 ? REJ0_FRAME : REJ1_FRAME, 5);
          return -1;
        }
        ll_stats.total_frames++;
        ll_stats.total_bytes_read += payloadSize + 6;
        readyFor = (readyFor + 1) % 2;
        memcpy(packet, frame + 4, payloadSize);
        send_frame(readyFor == 1 ? RR1_FRAME : RR0_FRAME, 5);
        return payloadSize;
      }
      i++;
      if (i > MAX_PAYLOAD_SIZE + 6) {
        printf("[RX] Frame too large. size=%d\n", i);
        return -1;
      }
    }
  }
  return -1;
}

int llread_frame(unsigned char *packet) {
  usleep(T_PROP * 2000); // ongoing and outgoing delay
  switch (readyFor) {
  case 1:
    return read_packet(packet, I1_HEADER);
  case 0:
  default:
    return read_packet(packet, I0_HEADER);
  }
}