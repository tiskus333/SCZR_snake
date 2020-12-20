#include "Pipe.h"

void sendTimestamp(const int64_t *p_data, size_t data_size, const int &pipe_write_fd)
{
  write(pipe_write_fd, p_data, data_size);
}

void receiveTimestamp(int64_t *p_write_to, size_t data_size, const int &pipe_read_fd)
{
  read(pipe_read_fd, p_write_to, data_size);
}

void sendFrame(const unsigned char *p_data, size_t data_size, const int &pipe_write_fd)
{
  write(pipe_write_fd, p_data, data_size);
}
void receiveFrame(unsigned char *p_write_to, size_t data_size, const int &pipe_read_fd)
{
  read(pipe_read_fd, p_write_to, data_size);
}