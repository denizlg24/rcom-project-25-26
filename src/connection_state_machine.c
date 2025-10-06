#include "connection_state_machine.h"


static LinkLayerRole currentRole;

void initializeRole(LinkLayerRole role) {
    currentRole = role;
}

State transitionState(State currState, unsigned char byte, unsigned char* setMessage, LinkLayerRole role) {
    switch (currState) {
    case START:
        if (byte == FLAG) {
            setMessage[0] = byte;
            return FLAG_RCV;
        }
        break;
    case FLAG_RCV:
        if (byte == FLAG) return FLAG_RCV;
        if (role == LlTx) {
            if (byte == SENDER_A) {
                setMessage[1] = byte;
                return A_RCV;
            }
        }
        else if (role == LlRx) {
            if (byte == RECEIVER_A) {
                setMessage[1] = byte;
                return A_RCV;
            }
        }
        return START;
    case A_RCV:
        if (byte == FLAG) return FLAG_RCV;
        if (role == LlTx) {
            if (byte == SET) {
                setMessage[2] = byte;
                return C_RCV;
            }
        }
        else if (role == LlRx) {
            if (byte == UA) {
                setMessage[2] = byte;
                return C_RCV;
            }
        }
        return START;
    case C_RCV:
        if (byte == FLAG) return FLAG_RCV;
        if (role == LlTx) {
            if (byte == SENDER_BCC1) {
                setMessage[3] = byte;
                return BCC_OK;
            }
        }
        else if (role == LlRx) {
            if (byte == RECEIVER_BCC1) {
                setMessage[3] = byte;
                return BCC_OK;
            }
        }
        return START;
    case BCC_OK:
        if (byte == FLAG) {
            setMessage[4] = byte;
            return STOP;
        }
        return START;
    default:
        return STOP;
    }
    return currState;
}

int processStateMachine(State* currState, unsigned char byte, unsigned char* setMessage, int* breakEarly, LinkLayerRole role) {
    if (*currState == STOP || *breakEarly != 0) {
        return 0;
    }

    State newState = transitionState(*currState, byte, setMessage, role);

    if (newState != *currState) {
        *currState = newState;
    }

    return 1;
}
