#ifndef _LINK_LAYER_STATS_H_
#define _LINK_LAYER_STATS_H_

typedef struct {
    long total_bytes_sent;
    long total_bytes_received;
    long total_data_bytes_sent;
    long total_data_bytes_received;
} LinkLayerStats;

extern LinkLayerStats ll_stats;

void reset_link_layer_stats();

#endif