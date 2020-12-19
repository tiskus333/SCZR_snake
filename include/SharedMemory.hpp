#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <stdio.h>
#include <stdlib.h>

#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>    // shared memory
#include <sys/mman.h> //

#include <algorithm>
#include <string.h>

#include <opencv2/core.hpp>

const size_t DATA_SIZE = 640 * 480 * 3;

typedef struct SharedMemory {
  sem_t sem_read;
  sem_t sem_write;
  size_t size;
  unsigned char frame[DATA_SIZE + 1];

  void sendToSharedMemory(const unsigned char *p_data, size_t data_size,
                          const int offset = 0);
  void receiveFromSharedMemory(unsigned char *p_write_to, size_t data_size,
                               const int offset = 0);
  void readFrame(const cv::Mat &frame);
  void writeFrame(const cv::Mat &frame);
  void readKey(unsigned char &key);
  void writeKey(const unsigned char &key);
} sh_m;

void createSharedMemory(const char *path_name);
sh_m *openSharedMemory(const char *path_name);

#endif