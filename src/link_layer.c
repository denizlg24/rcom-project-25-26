// Link layer protocol implementation

#include "serial_port.h"
#include <signal.h>
#include <unistd.h>
#include "connection_state_machine.h"
#include <stdlib.h>
// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

//LLOPEN
State currState = START;
int breakEarly = FALSE;
int tries = 0;
int trying = FALSE;
int UAReceived = FALSE;

////////////////////////////////////////////////
// LLOPEN - TRANSMITER
////////////////////////////////////////////////
void tx_alarm_handler(int signal)
{
    trying = FALSE;
    tries++;
    breakEarly = TRUE;
}


int send_set_signal() {
    unsigned char setFrame[5] = { FLAG, SENDER_A, SET, SENDER_BCC1, FLAG };

    if (writeBytesSerialPort(setFrame, 5) < 0) {
        perror("writeBytesSerialPort");
        exit(-1);
    }

    return 0;
}

int read_ua() {
    unsigned char byte;
    unsigned char message[5];

    while (currState != STOP && breakEarly == 0)
    {
        int bytes = readByteSerialPort(&byte);
        if (bytes == -1)
        {
            perror("readByteSerialPort");
            if (closeSerialPort() < 0)
            {
                perror("closeSerialPort");
            }
            exit(-1);
        }

        if (bytes > 0)
        {
            if (!processStateMachine(&currState, byte, message, &breakEarly, LlTx))
            {
                break;
            }
        }
    }

    return 0;
}

int sendAndWaitUA()
{

    printf("Starting try=%d\n", tries);
    breakEarly = 0;
    send_set_signal();
    read_ua();
    if (currState == STOP)
    {
        UAReceived = TRUE;
        alarm(0);
    }

    return 0;
}

int llopen_tx(LinkLayer connectionParameters) {
    struct sigaction act = { 0 };
    act.sa_handler = &tx_alarm_handler;
    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(-1);
    }

    while (tries < connectionParameters.nRetransmissions && UAReceived == FALSE)
    {
        if (trying == FALSE)
        {
            alarm(connectionParameters.timeout);
            trying = TRUE;
            sendAndWaitUA();
        }
    }
    if (UAReceived == 0)
    {
        printf("Timed Out after tries=%d\n", connectionParameters.nRetransmissions);
        if (closeSerialPort() < 0)
        {
            perror("closeSerialPort");
            exit(-1);
        }
        return 0;
    }
    printf("Connected successfuly.\n");
    return 0;
}

////////////////////////////////////////////////
// LLOPEN - RECEIVER
////////////////////////////////////////////////

int send_ua_signal() {
    unsigned char uaFrame[5] = { FLAG, RECEIVER_A, UA, RECEIVER_BCC1, FLAG };

    if (writeBytesSerialPort(uaFrame, 5) < 0) {
        perror("writeBytesSerialPort");
        exit(-1);
    }

    return 0;
}

int read_set() {
    unsigned char byte;
    unsigned char message[5];

    while (currState != STOP)
    {
        int bytes = readByteSerialPort(&byte);
        if (bytes == -1)
        {
            perror("readByteSerialPort");
            if (closeSerialPort() < 0)
            {
                perror("closeSerialPort");
            }
            exit(-1);
        }

        if (bytes > 0)
        {
            if (!processStateMachine(&currState, byte, message, &breakEarly, LlRx))
            {
                break;
            }
        }
    }

    return 0;
}

int llopen_rx(LinkLayer connectionParameters) {

    read_set();
    send_set_signal();
    printf("Connected\n");
    return 0;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    if (openSerialPort(connectionParameters.serialPort, connectionParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        exit(-1);
    }

    printf("Serial port %s opened\n", connectionParameters.serialPort);
    if (connectionParameters.role == LlTx) {
        return llopen_tx(connectionParameters);
    }
    else {
        return llopen_rx(connectionParameters);
    }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char* buf, int bufSize)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char* packet)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose()
{
    // TODO: Implement this function

    return 0;
}
