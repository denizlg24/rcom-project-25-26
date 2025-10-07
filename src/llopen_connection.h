#ifndef LLOPEN_CONNECTION_H
#define LLOPEN_CONNECTION_H

#include "link_layer.h"

int llopen_tx_connection(LinkLayer connectionParameters);
int llopen_rx_connection(LinkLayer connectionParameters);

#endif