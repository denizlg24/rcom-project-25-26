#ifndef LLWRITE_FRAME_H
#define LLWRITE_FRAME_H

#include "link_layer.h"

int llwrite_frame(const unsigned char *buf, int bufSize,LinkLayer connectionParameters);

#endif