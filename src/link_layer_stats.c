#include "link_layer_stats.h"

LinkLayerStats ll_stats = {0};

void reset_link_layer_stats() {
  ll_stats.total_bytes_sent = 0;
  ll_stats.total_bytes_received = 0;
  ll_stats.total_data_bytes_sent = 0;
  ll_stats.total_data_bytes_received = 0;
}