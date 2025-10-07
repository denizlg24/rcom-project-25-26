#define _XOPEN_SOURCE 700 // SILENCE SIGNAL ERROR

#include "llwrite_frame.h"
#include "frame_helpers.h"
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

void build_frame(int _ns, const unsigned char *buf, size_t bufSize,
                 unsigned char *frame, size_t *frameSize) {
  const unsigned char *header = (_ns == 0) ? I0_HEADER : I1_HEADER;
  memcpy(frame, header, 4);
  memcpy(frame + 4, buf, bufSize);
  frame[4 + bufSize] = compute_bcc2(buf, bufSize);
  frame[4 + bufSize + 1] = FLAG;
  *frameSize = bufSize + 6;
}

static int send_and_wait(LinkLayer connectionParameters,
                         const unsigned char *frame,
                         const unsigned char *expectedResponse) {
  printf("[TX] Attempt %d/%d: Sending frame...\n", tries + 1,
         connectionParameters.nRetransmissions);

  alarm(connectionParameters.timeout);
  send_frame(frame, sizeof(frame));
  abortSignal = 0;
  printf("[TX] Attempt %d/%d: Waiting for RR...\n", tries + 1,
         connectionParameters.nRetransmissions);
  int success = read_frame(expectedResponse, &abortSignal);
  alarm(0);
  return success;
}

int llwrite_frame(const unsigned char *buf, int bufSize,
                  LinkLayer connectionParameters) {

  unsigned char frame[bufSize + 6];
  size_t frameSize;
  build_frame(ns, buf, bufSize, frame, &frameSize);

  struct sigaction act = {0};
  act.sa_handler = tx_alarm_handler;
  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  tries = 0;
  abortSignal = 0;

  unsigned char RRReceived = 0;
  const unsigned char *expectedRR = ns == 0 ? RR1_FRAME : RR0_FRAME;
  while (tries < connectionParameters.nRetransmissions && !RRReceived) {
    RRReceived = send_and_wait(connectionParameters, frame, expectedRR);
  }

  if (!RRReceived) {
    printf("[TX] Timeout! No RR received after %d/%d attempts.\n", tries,
           connectionParameters.nRetransmissions);
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
      exit(EXIT_FAILURE);
    }
    return -1;
  }
  ns = (ns + 1) % 2;
  return 0;
}