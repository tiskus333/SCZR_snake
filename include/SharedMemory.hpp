#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <semaphore.h>

#include <sys/mman.h> //
#include <fcntl.h>    // shared memory

#define FRAME_SIZE 16

typedef struct SharedMemory
{
    sem_t sem_read;
    sem_t sem_write;
    char frame[FRAME_SIZE];
} sh_m;

void createSharedMemory(const char *path_name);
sh_m *openSharedMemory(const char *path_name);

#endif