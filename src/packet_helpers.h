#ifndef PACKET_HELPERS_H
#define PACKET_HELPERS_H

int build_control_packet(unsigned char* packet,const unsigned char* filename,int filename_length,int file_size);
int build_end_packet(unsigned char* packet,const unsigned char* filename,int filename_length,int file_size);
int build_data_packet(unsigned char* packet, const unsigned char* data, int data_length);
long get_file_size(const char* filename);
void parse_control_packet(const unsigned char* packet, int length, long* file_size, char* filename);
int parse_data_packet(const unsigned char* packet, int length, unsigned char* data, int* data_length);
#endif