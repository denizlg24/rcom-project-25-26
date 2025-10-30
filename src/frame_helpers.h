#ifndef FRAME_HELPERS_H
#define FRAME_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#define FLAG ((unsigned char)0x7E)
#define A_TX ((unsigned char)0x03)
#define A_RX ((unsigned char)0x01)
#define C_SET ((unsigned char)0x03)
#define C_UA ((unsigned char)0x07)
#define C_DISC ((unsigned char)0x0B)

#define C_RR0 ((unsigned char)0xAA)
#define C_RR1 ((unsigned char)0xAB)

#define C_REJ0 ((unsigned char)0x54)
#define C_REJ1 ((unsigned char)0x55)

#define BCC1(A, C) ((unsigned char)((A) ^ (C)))

#define C_I0 ((unsigned char)0x00)
#define C_I1 ((unsigned char)0x80)

#define ESC ((unsigned char)0x7D)
#define ESC_FLAG ((unsigned char)0x5E)
#define ESC_ESC ((unsigned char)0x5D)

static const unsigned char SET_FRAME[5] = {FLAG, A_TX, C_SET, BCC1(A_TX, C_SET),
                                           FLAG};
static const unsigned char UA_FRAME[5] = {FLAG, A_RX, C_UA, BCC1(A_RX, C_UA),
                                          FLAG};
static const unsigned char TX_UA_FRAME[5] = {FLAG, A_TX, C_UA, BCC1(A_TX, C_UA),
                                          FLAG};
static const unsigned char DISC_TX_FRAME[5] = {FLAG, A_TX, C_DISC,
                                               BCC1(A_TX, C_DISC), FLAG};
static const unsigned char DISC_RX_FRAME[5] = {FLAG, A_RX, C_DISC,
                                               BCC1(A_RX, C_DISC), FLAG};

static const unsigned char RR0_FRAME[5] = {FLAG, A_RX, C_RR0, BCC1(A_RX, C_RR0),
                                           FLAG};
static const unsigned char RR1_FRAME[5] = {FLAG, A_RX, C_RR1, BCC1(A_RX, C_RR1),
                                           FLAG};
static const unsigned char REJ0_FRAME[5] = {FLAG, A_RX, C_REJ0,
                                            BCC1(A_RX, C_REJ0), FLAG};
static const unsigned char REJ1_FRAME[5] = {FLAG, A_RX, C_REJ1,
                                            BCC1(A_RX, C_REJ1), FLAG};
static const unsigned char I0_HEADER[4] = {FLAG, A_TX, C_I0, BCC1(A_TX, C_I0)};
static const unsigned char I1_HEADER[4] = {FLAG, A_TX, C_I1, BCC1(A_TX, C_I1)};

unsigned char compute_bcc2(const unsigned char *data, size_t length);

int send_frame(const unsigned char *frame, size_t size);
int read_frame(const unsigned char *expected, unsigned char *abortSignal);

#endif