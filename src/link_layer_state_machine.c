#include "link_layer_state_machine.h"

State transitionState(State currState, unsigned char byte,
                      const unsigned char *expectedHeader, int *bcc1_errors) {
  switch (currState) {
  case START:
    if (byte == FLAG) {
      return FLAG_RCV;
    }
    return START;

  case FLAG_RCV:
    if (byte == FLAG)
      return FLAG_RCV;

    if (byte == expectedHeader[1]) {
      return A_RCV;
    }

    return START;

  case A_RCV:
    if (byte == FLAG)
      return FLAG_RCV;

    if (byte == expectedHeader[2]) {
      return C_RCV;
    }

    return START;

  case C_RCV:
    if (byte == expectedHeader[3]) {
      return BCC_OK;
    } else {
      if (bcc1_errors != NULL) {
        (*bcc1_errors)++;
      }
      if (byte == FLAG)
        return FLAG_RCV;
      return START;
    }
  case BCC_OK:
    if (byte == expectedHeader[4]) {
      return STOP;
    }
    return START;

  default:
    return START;
  }
}

int processStateMachine(State *currState, unsigned char byte,
                        unsigned char *abortSignal,
                        const unsigned char *expectedHeader, int *bcc1_errors) {

  if (*currState == STOP || *abortSignal != 0) {
    return 0;
  }

  State newState =
      transitionState(*currState, byte, expectedHeader, bcc1_errors);

  if (newState != *currState) {
    *currState = newState;
  }

  return 1;
}
