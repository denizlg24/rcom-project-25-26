#define _XOPEN_SOURCE 700 // SILENCE SIGNAL ERROR
// Link layer protocol implementation

#include "connection_state_machine.h"
#include "serial_port.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

State currState = START;
int tries = 0;
int UAReceived = FALSE;
int breakEarly = FALSE;

////////////////////////////////////////////////
// TRANSMITTER
////////////////////////////////////////////////
void tx_alarm_handler(int signal) {
  tries++;
  breakEarly = TRUE;
}

static int send_frame(const unsigned char *frame, size_t size) {
  if (writeBytesSerialPort(frame, size) < 0) {
    perror("writeBytesSerialPort");
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
    }
    exit(EXIT_FAILURE);
  }
  return 0;
}

static int send_set() {
  unsigned char setFrame[5] = {FLAG, SENDER_A, SET, SENDER_BCC1, FLAG};
  return send_frame(setFrame, sizeof(setFrame));
}

static int send_ua() {
  unsigned char uaFrame[5] = {FLAG, RECEIVER_A, UA, RECEIVER_BCC1, FLAG};
  return send_frame(uaFrame, sizeof(uaFrame));
}

int read_frame(LinkLayerRole role) {
  unsigned char byte;
  unsigned char message[5];
  currState = START;
  while (currState != STOP && !breakEarly) {
    int bytes = readByteSerialPort(&byte);
    if (bytes == -1) {
      perror("readByteSerialPort");
      if (closeSerialPort() < 0) {
        perror("closeSerialPort");
      }
      exit(EXIT_FAILURE);
    }

    if (bytes > 0 &&
        !processStateMachine(&currState, byte, message, &breakEarly, role)) {
      break;
    }
  }
  return currState == STOP;
}

int send_and_wait_ua(LinkLayer connectionParameters) {
  printf("[TX] Attempt %d/%d: Sending SET frame...\n", tries + 1,
         connectionParameters.nRetransmissions);
  alarm(connectionParameters.timeout);
  send_set();
  breakEarly = FALSE;
  int success = read_frame(connectionParameters.role);
  alarm(0);
  return success;
}

int llopen_tx(LinkLayer connectionParameters) {
  struct sigaction act = {0};
  act.sa_handler = tx_alarm_handler;
  if (sigaction(SIGALRM, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  while (tries < connectionParameters.nRetransmissions && !UAReceived) {
    UAReceived = send_and_wait_ua(connectionParameters);
  }

  if (!UAReceived) {
    printf("[TX] Timeout! No UA received after %d/%d attempts.\n", tries,
           connectionParameters.nRetransmissions);
    if (closeSerialPort() < 0) {
      perror("closeSerialPort");
    }
    return -1;
  }

  printf("[TX] UA received successfully! Connection established.\n");
  return 0;
}

////////////////////////////////////////////////
// RECEIVER
////////////////////////////////////////////////
int llopen_rx(LinkLayer connectionParameters) {
  printf("[RX] Waiting for SET frame from transmitter...\n");
  if (read_frame(connectionParameters.role)) {
    printf("[RX] SET frame received. Sending UA response...\n");
    send_ua();
    printf("[RX] Connection established successfully.\n");
    return 0;
  }
  printf("[RX] ERROR: Failed to receive SET frame. Connection aborted.\n");
  if (closeSerialPort() < 0) {
    perror("closeSerialPort");
  }
  return -1;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {
  if (openSerialPort(connectionParameters.serialPort,
                     connectionParameters.baudRate) < 0) {
    perror("openSerialPort");
    exit(EXIT_FAILURE);
  }

  printf("Serial port %s opened\n", connectionParameters.serialPort);

  if (connectionParameters.role == LlTx) {
    return llopen_tx(connectionParameters);
  } else {
    return llopen_rx(connectionParameters);
  }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {
  // TODO: Implement this function

  return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
  // TODO: Implement this function

  return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose() {
  // TODO: Implement this function

  return 0;
}
