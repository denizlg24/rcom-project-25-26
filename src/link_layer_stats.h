#ifndef _LINK_LAYER_STATS_H_
#define _LINK_LAYER_STATS_H_

#include <time.h>
#define FILESIZE 10968 // in bytes
#define T_PROP 0 // in milliseconds
#define BCC1_ERROR_PROB 0 // in percentage
#define BCC2_ERROR_PROB 0 // in percentage

typedef struct {
  long total_bytes_read;
  long total_frames;
  long error_frames;
  long retransmissions;
  clock_t start_time;
  clock_t end_time;
} LinkLayerStats;

extern LinkLayerStats ll_stats;

double get_a(int baudrate, int frame_size);
double get_fer();

double measured_bit_rate();

double theoretical_effeciency(int baudrate, int frame_size);

double measured_effeciency(int baudrate);

#endif