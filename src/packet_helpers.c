#include "packet_helpers.h"
#include <stdio.h>
#include <string.h>


long get_file_size(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    return size;
}

int build_control_packet(unsigned char* packet,const unsigned char* filename,int filename_length,int file_size){
    int index = 0;
    packet[index++] = 1;

    packet[index++] = 0;

    int temp_size = file_size;
    int L1 = 0;
    do {
        L1++;
        temp_size >>= 8;
    } while (temp_size > 0);

    packet[index++] = L1;

    for (int i = L1 - 1; i >= 0; i--) {
        packet[index++] = (file_size >> (8 * i)) & 0xFF;
    }

    packet[index++] = 1;
    packet[index++] = filename_length;

    for (int i = 0; i < filename_length; i++) {
        packet[index++] = filename[i];
    }
    return index;
}

int build_data_packet(unsigned char* packet, const unsigned char* data, int data_length) {
    int index = 0;
    packet[index++] = 2;
    packet[index++] = (data_length >> 8) & 0xFF;
    packet[index++] = data_length & 0xFF;
    for (int i = 0; i < data_length; i++)
        packet[index++] = data[i];
    return index;
}

void parse_control_packet(const unsigned char* packet, int length, long* file_size, char* filename) {
    int index = 0;

    if (packet[index++] != 1) {
        return;
    }

    if (packet[index++] != 0) {
        return;
    }

    int L1 = packet[index++];
    *file_size = 0;

    for (int i = 0; i < L1; i++) {
        *file_size = (*file_size << 8) | packet[index++];
    }

    if (packet[index++] != 1) {
        return;
    }

    int L2 = packet[index++];
    memcpy(filename, &packet[index], L2);
    filename[L2] = '\0';
}

int parse_data_packet(const unsigned char* packet, int length, unsigned char* data, int* data_length) {
    int index = 0;

    if (packet[index++] != 2) {
        return -1;
    }

    unsigned char L2 = packet[index++];
    unsigned char L1 = packet[index++];
    *data_length = (L1 << 8) | L2;

    if (*data_length > length - 3) {
        return -1;
    }

    memcpy(data, &packet[index], *data_length);
    return 0;
}


