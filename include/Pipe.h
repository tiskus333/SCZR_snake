#ifndef PIPE_H
#define PIPE_H

#include <unistd.h>
#include <sys/types.h>

#include <algorithm>

void sendTimestamp(const int64_t *p_data, size_t data_size, const int &pipe_write_fd);
void receiveTimestamp(int64_t *p_write_to, size_t data_size, const int &pipe_read_fd);

void sendFrame(const unsigned char *p_data, size_t data_size, const int &pipe_write_fd);
void receiveFrame(unsigned char *p_write_to, size_t data_size, const int &pipe_read_fd);

#endif