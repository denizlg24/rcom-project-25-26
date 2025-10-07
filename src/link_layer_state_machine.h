#ifndef LINK_LAYER_STATE_MACHINE_H
#define LINK_LAYER_STATE_MACHINE_H

#include "frame_helpers.h"
#include <stdio.h>

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } State;

State transitionState(State currState, unsigned char byte,
                      const unsigned char *expectedHeader);
int processStateMachine(State *currState, unsigned char byte, unsigned char *abortSignal,
                        const unsigned char *expectedHeader);

#endif