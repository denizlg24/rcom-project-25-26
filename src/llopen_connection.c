#define _XOPEN_SOURCE 700 // SILENCE SIGNAL ERROR

#include "llopen_connection.h"
#include "frame_helpers.h"
#include "link_layer_state_machine.h"
#include "serial_port.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int tries = 0;
static unsigned char abortSignal = 0;

static void tx_alarm_handler(int signal) {
  tries++;
  abortSignal = 1;
}

////////////////////////////////////////////////
// TRANSMITTER
////////////////////////////////////////////////
static int send_and_wait_ua(LinkLayer connectionParameters) {
  printf("[TX] Attempt %d/%d: Sending SET frame...\n", tries + 1,
         connectionParameters.nRetransmissions);

  alarm(connectionParameters.timeout);
  send_frame(SET_FRAME, sizeof(SET_FRAME));
  abortSignal = 0;

  int success = read_frame(UA_FRAME, &abortSignal);
  alarm(0);
  return success;
}

int llopen_tx_connection(LinkLayer connectionParameters) {
  struct sigaction act = {0};
  act.sa_handler = tx_alarm_handler;

  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  tries = 0;
  unsigned char UAReceived = 0;

  while (tries < connectionParameters.nRetransmissions && !UAReceived) {
    UAReceived = send_and_wait_ua(connectionParameters);
  }

  if (!UAReceived) {
    printf("[TX] Timeout! No UA received after %d/%d attempts.\n", tries,
           connectionParameters.nRetransmissions);
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
      exit(EXIT_FAILURE);
    }
    return -1;
  }

  printf("[TX] UA received successfully! Connection established.\n");
  return 0;
}

////////////////////////////////////////////////
// RECEIVER
////////////////////////////////////////////////
int llopen_rx_connection(LinkLayer connectionParameters) {
  printf("[RX] Waiting for SET frame from transmitter...\n");
  unsigned char dummyAbort = 0;
  if (read_frame(SET_FRAME, &dummyAbort)) {
    printf("[RX] SET frame received. Sending UA response...\n");
    send_frame(UA_FRAME, sizeof(UA_FRAME));
    printf("[RX] Connection established successfully.\n");
    return 0;
  }

  printf("[RX] ERROR: Failed to receive SET frame. Connection aborted.\n");
  if (closeSerialPort() < 0) {
    perror("closeSerialPort");
    exit(EXIT_FAILURE);
  }
  return -1;
}