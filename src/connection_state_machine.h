#ifndef CONNECTION_STATE_MACHINE_H
#define CONNECTION_STATE_MACHINE_H

#include "link_layer.h"
#include <stdio.h>

#define FLAG ((unsigned char)0x7E)
#define SENDER_A ((unsigned char)0x03)
#define RECEIVER_A ((unsigned char)0x01)
#define SET ((unsigned char)0x03)
#define UA ((unsigned char)0x07)
#define SENDER_BCC1 ((unsigned char)0x00)
#define RECEIVER_BCC1 ((unsigned char)0x06)

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } State;

void initializeRole(LinkLayerRole role);

State transitionState(State currState, unsigned char byte,
                      unsigned char *setMessage, LinkLayerRole role);
int processStateMachine(State *currState, unsigned char byte,
                        unsigned char *setMessage, int *breakEarly,
                        LinkLayerRole role);

#endif