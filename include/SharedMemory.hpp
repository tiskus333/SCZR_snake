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

typedef struct SharedFrame {
  sem_t sem_read;
  sem_t sem_write;
  size_t size;
  unsigned char frame[DATA_SIZE];

  void sendToSharedMemory(const unsigned char *p_data, size_t data_size);
  void receiveFromSharedMemory(unsigned char *p_write_to, size_t data_size );
  void readFrame(const cv::Mat &frame);
  void writeFrame(const cv::Mat &frame);
} sh_m;

void createSharedMemory(const char *path_name);
sh_m *openSharedMemory(const char *path_name);

typedef struct SharedGameState {
    sem_t sem_read;
    sem_t sem_write;
    sem_t try_read;
    sem_t resource;
    int read_count;
    int write_count;
    //
    char key_pressed;

    char readKey();
    void writeKey(const char &key);
} gm_st;

gm_st *createSharedGameState(const char *path_name);
gm_st *openSharedGameState(const char *path_name);

#endif