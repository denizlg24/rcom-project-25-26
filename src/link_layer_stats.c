#include "link_layer_stats.h"

LinkLayerStats ll_stats = {
    .total_bytes_read = 0,
    .total_frames = 0,
    .error_frames = 0,
    .retransmissions = 0,
    .start_time = 0,
    .end_time = 0
};


double get_a(int baudrate, int frame_size) {
  return ((double)(T_PROP/1000)) /
         ((double)frame_size * 8.0 / (double)baudrate);
}

double get_fer() {
  double bcc1_error_prob = (double)BCC1_ERROR_PROB / 100.0;
  double bcc2_error_prob = (double)BCC2_ERROR_PROB / 100.0;
  return bcc1_error_prob + bcc2_error_prob * (1 - bcc1_error_prob);
}

double measured_bit_rate() {
  return ((double)(FILESIZE * 8)) /
         ((double)(ll_stats.end_time - ll_stats.start_time) / CLOCKS_PER_SEC);
}

double theoretical_effeciency(int baudrate, int frame_size) {
  return (1 - get_fer()) / (1 + (2 * get_a(baudrate, frame_size)));
}

double measured_effeciency(int baudrate) {
  return measured_bit_rate() / (double)baudrate;
}