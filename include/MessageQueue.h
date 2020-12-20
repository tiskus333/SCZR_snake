#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <unistd.h>
#include <sys/types.h>

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>

const long MSGQ_MAX_MSG = 10;       // maximum 10

const long PAYLOAD = 4096;

void msgqSendFrame(int msgq_des, char *p_data, const size_t data_size);
void msgqReceiveFrame(int msgq_des, char *p_data, const size_t data_size);

#endif