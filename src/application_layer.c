// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "link_layer_stats.h"
#include "packet_helpers.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {
  LinkLayer options = {0};

  strcpy(options.serialPort, serialPort);

  options.role = strcmp("tx", role) == 0 ? LlTx : LlRx;

  options.baudRate = baudRate;
  options.nRetransmissions = nTries;
  options.timeout = timeout;
  if (llopen(options) < 0) {
    exit(0);
  }

  if (options.role == LlTx) {
    long file_size = get_file_size(filename);
    if (file_size < 0) {
      exit(llclose());
    }

    unsigned char control_packet[256];
    int control_packet_size =
        build_control_packet(control_packet, (const unsigned char *)filename,
                             strlen(filename), file_size);
    llwrite(control_packet, control_packet_size);

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
      perror("Error opening file");
      exit(llclose());
    }

    int max_data_bytes = MAX_PAYLOAD_SIZE - 3;
    unsigned char file_data[max_data_bytes];
    unsigned char data_packet[MAX_PAYLOAD_SIZE];
    size_t bytes_read;
    int packet_size;
    while ((bytes_read = fread(file_data, 1, max_data_bytes, fp)) > 0) {
      packet_size = build_data_packet(data_packet, file_data, bytes_read);
      int wrote = llwrite(data_packet, packet_size);
      if (wrote < 0) {
        fclose(fp);
        exit(llclose());
      }
    }
    fclose(fp);

    unsigned char end_packet[256];
    int end_packet_size =
        build_end_packet(end_packet, (const unsigned char *)filename,
                         strlen(filename), file_size);
    llwrite(end_packet, end_packet_size);
    int close_status = llclose();
    printf("[TX] --- STATS ---\n");
    printf("[TX] Total frames: %ld\n", ll_stats.total_frames);
    printf("[TX] Retransmissions: %ld\n", ll_stats.retransmissions);
    printf("[TX] Total time: %f seconds\n",
           (double)(ll_stats.end_time - ll_stats.start_time) / CLOCKS_PER_SEC);
    printf("[TX] Measured efficiency: %f\n", measured_effeciency(options.baudRate));
    printf("[TX] Theoretical efficiency: %f\n",
           theoretical_effeciency(options.baudRate, MAX_PAYLOAD_SIZE));
    exit(close_status);
  } else {
    unsigned char *packet = (unsigned char *)malloc(MAX_PAYLOAD_SIZE);
    int read_size = 0;
    FILE *fp = NULL;
    unsigned char data_buffer[MAX_PAYLOAD_SIZE];
    while (1) {
      while ((read_size = llread(packet)) < 0) {
        if (read_size == -2) {
          memset(packet, 0, MAX_PAYLOAD_SIZE);
          if (fp) {
            fclose(fp);
            remove(filename);
          }
        }
      }
      if (read_size == 0)
        break;
      if (packet[0] == 1) {
        long file_size;
        char filename_sender[256];
        parse_control_packet(packet, read_size, &file_size, filename_sender);
        printf("[RX] File name: %s\n", filename_sender);
        printf("[RX] File size: %ld bytes\n", file_size);
        fp = fopen(filename, "wb");
      } else if (packet[0] == 2) {
        if (!fp) {
          continue;
        }
        int data_len = 0;
        if (parse_data_packet(packet, read_size, data_buffer, &data_len) == 0) {
          fwrite(data_buffer, 1, data_len, fp);
          printf("[RX] Wrote %d bytes to file.\n", data_len);
        }
      } else if (packet[0] == 3) {
        if (fp) {
          fclose(fp);
        }
        int close_status = llclose();
        printf("[RX] --- STATS ---\n");
        printf("[RX] Total frames: %ld\n", ll_stats.total_frames);
        printf("[RX] Error frames: %ld\n", ll_stats.error_frames);
        printf("[RX] Read bytes: %ld\n", ll_stats.total_bytes_read);
        printf("[RX] Total time: %f seconds\n",
               (double)(ll_stats.end_time - ll_stats.start_time) /
                   CLOCKS_PER_SEC);
        printf("[RX] Measured bit rate: %f\n", measured_bit_rate());
        exit(close_status);
      }
    }
  }
}
