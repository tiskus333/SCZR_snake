#ifndef PIPE_H
#define PIPE_H

#include <unistd.h>
#include <sys/types.h>

template <typename T>
void pipeSend(const int pipe_write_fd, T *p_data, size_t data_size)
{
  write(pipe_write_fd, p_data, data_size);
}

template <typename T>
void pipeReceive(const int pipe_read_fd, T *p_write_to, size_t data_size)
{
  read(pipe_read_fd, p_write_to, data_size);
}
#endif