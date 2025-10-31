#define _XOPEN_SOURCE 700 // SILENCE SIGNAL ERROR

#include "llclose_connection.h"
#include "frame_helpers.h"
#include "link_layer_state_machine.h"
#include "link_layer_stats.h"
#include "serial_port.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static unsigned char txAbortSignal = 0;
static unsigned char rxAbortSignal = 0;

static void tx_alarm_handler(int signal) { txAbortSignal = 1; }
static void rx_alarm_handler(int signal) { rxAbortSignal = 1; }

int close_tx_connection(LinkLayer connectionParameters) {
  struct sigaction act = {0};
  act.sa_handler = tx_alarm_handler;
  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(-1);
  }
  alarm(3);
  send_frame(DISC_TX_FRAME, 5);
  ll_stats.total_frames++;
  if (read_frame(DISC_RX_FRAME, &txAbortSignal)) {
    printf("[TX] Got RX_DISC, disconnecting...\n");
    send_frame(TX_UA_FRAME, 5);
    ll_stats.total_frames++;
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
      exit(-1);
    }
    return 0;
  } else {
    printf("[TX] Didn't get RX_DISC, disconnecting...\n");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
      exit(-1);
    }
    return 0;
  }
  return 0;
}

int close_rx_connection(LinkLayer connectionParameters) {
  struct sigaction act = {0};
  act.sa_handler = rx_alarm_handler;
  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(-1);
  }
  alarm(3);
  send_frame(DISC_RX_FRAME, 5);
  ll_stats.total_frames++;
  ll_stats.total_bytes_read += 5;
  if (read_frame(TX_UA_FRAME, &rxAbortSignal)) {
    printf("[RX] Got UA, disconnecting...\n");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
      exit(-1);
    }
    return 0;
  } else {
    printf("[RX] Didn't get UA, disconnecting...\n");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
      exit(-1);
    }
    return 0;
  }
  return 0;
}

int llclose_connection(LinkLayer connectionParameters) {
  if (connectionParameters.role == LlRx) {
    int closed_rx = close_rx_connection(connectionParameters);
    ll_stats.end_time = clock();
    return closed_rx;
  } else {
    int closed_tx = close_tx_connection(connectionParameters);
     ll_stats.end_time = clock();
    return closed_tx;
  }
}