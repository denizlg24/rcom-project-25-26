#define _XOPEN_SOURCE 700 // SILENCE SIGNAL ERROR

#include "llwrite_frame.h"
#include "frame_helpers.h"
#include "link_layer_state_machine.h"
#include "link_layer_stats.h"
#include "serial_port.h"
#include "string.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


unsigned char ns = 0;
int tries = 0;
static unsigned char abortSignal = 0;

static void tx_alarm_handler(int signal) {
  tries++;
  abortSignal = 1;
}

int stuff_bytes(const unsigned char *input, int inputSize,
                unsigned char *output) {
  int j = 0;

  for (int i = 0; i < inputSize; i++) {
    if (input[i] == FLAG) {
      output[j++] = ESC;
      output[j++] = ESC_FLAG;
    } else if (input[i] == ESC) {
      output[j++] = ESC;
      output[j++] = ESC_ESC;
    } else {
      output[j++] = input[i];
    }
  }

  return j;
}

void build_frame(const unsigned char *buf, int bufSize,
                 const unsigned char *header, unsigned char *frame,
                 int *frameSize) {
  int headerSize = 4;
  memcpy(frame, header, headerSize);

  unsigned char stuffedBuf[2 * bufSize + 2];
  int stuffedSize = stuff_bytes(buf, bufSize, stuffedBuf);

  memcpy(frame + headerSize, stuffedBuf, stuffedSize);

  unsigned char bcc2 = compute_bcc2(buf, bufSize);
  unsigned char bcc2Stuffed[2];
  int bcc2Size = stuff_bytes(&bcc2, 1, bcc2Stuffed);

  memcpy(frame + headerSize + stuffedSize, bcc2Stuffed, bcc2Size);

  frame[headerSize + stuffedSize + bcc2Size] = FLAG;
  *frameSize = headerSize + stuffedSize + bcc2Size + 1;
}

int read_response(const unsigned char *expected, unsigned char *abortSignal) {
  unsigned char byte;
  State successState = START;
  State rejectState = START;
  (*abortSignal) = 0;
  while (successState != STOP && rejectState != STOP && (*abortSignal) != 1) {
    int bytes = readByteSerialPort(&byte);
    if (bytes == -1) {
      perror("readByteSerialPort");
      if (closeSerialPort() < 0) {
        perror("closeSerialPort");
      }
      exit(EXIT_FAILURE);
    }

    if (bytes > 0) {
      int isSuccess = !processStateMachine(&successState, byte, abortSignal,
                                           expected, NULL);
      int rejected =
          !processStateMachine(&rejectState, byte, abortSignal,
                               ns == 0 ? REJ0_FRAME : REJ1_FRAME, NULL);
      if (isSuccess || rejected)
        break;
    }
  }
  if (rejectState == STOP) {
    return -1;
  }
  return successState == STOP;
}

int send_and_wait(LinkLayer connectionParameters, const unsigned char *frame,
                  int *frameSize, const unsigned char *expectedResponse) {
  printf("[TX] Attempt %d/%d: Sending frame...\n", tries + 1,
         connectionParameters.nRetransmissions);
  alarm(connectionParameters.timeout);
  if (tries != 0) {
    ll_stats.retransmissions++;
  }
  send_frame(frame, *frameSize);
  ll_stats.total_frames++;
  abortSignal = 0;
  printf("[TX] Attempt %d/%d: Waiting for RR%d...\n", tries + 1,
         connectionParameters.nRetransmissions, (ns + 1) % 2);
  int success = read_response(expectedResponse, &abortSignal);
  if (success > 0) {
    printf("[TX] Attempt %d/%d: Received RR%d\n", tries + 1,
           connectionParameters.nRetransmissions, (ns + 1) % 2);
    alarm(0);
  } else if (success == 0) {
    printf("[TX] Attempt %d/%d: Dind't receive RR%d\n", tries + 1,
           connectionParameters.nRetransmissions, (ns + 1) % 2);
  } else {
    printf("[TX] Attempt %d/%d: Got REJ%d\n", tries + 1,
           connectionParameters.nRetransmissions, ns);
    ll_stats.retransmissions++;
  }
  return success;
}

int llwrite_frame(const unsigned char *buf, int bufSize,
                  LinkLayer connectionParameters) {
  unsigned char frame[bufSize + 6];
  int frameSize;
  if (ns == 0) {
    build_frame(buf, bufSize, I0_HEADER, frame, &frameSize);
  } else {
    build_frame(buf, bufSize, I1_HEADER, frame, &frameSize);
  }

  struct sigaction act = {0};
  act.sa_handler = tx_alarm_handler;
  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  tries = 0;
  abortSignal = 0;

  int RRReceived = 0;
  const unsigned char *expectedRR = ns == 0 ? RR1_FRAME : RR0_FRAME;
  while (tries < connectionParameters.nRetransmissions && RRReceived < 1) {
    RRReceived =
        send_and_wait(connectionParameters, frame, &frameSize, expectedRR);
  }

  if (!RRReceived) {
    printf("[TX] Timeout! No RR received after %d/%d attempts.\n", tries,
           connectionParameters.nRetransmissions);
    return -1;
  }
  ns = (ns + 1) % 2;
  return 0;
}