#ifndef PIPE_H
#define PIPE_H

#include <unistd.h>
#include <sys/types.h>

#include <chrono>

void createPipe(int pipefd[2])
{
  if (pipe(pipefd) == -1)
  {
    perror("Cannot create pipe.");
    exit(EXIT_FAILURE);
  }
}

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

int64_t getTimestamp()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

#endif