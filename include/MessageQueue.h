#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

class MessageQueue {
  mqd_t msgq_des_;
  struct mq_attr attributes_;
  struct timespec timeout_;

  // constants
  const long PAYLOAD_SIZE = 4096;
  const long MSGQ_MAX_MSG = 10; // maximum 10
  time_t TIMEOUT = 10;

public:
  MessageQueue();
  void create(const char *msgq_file_name);
  void open(const char *msgq_file_name, const int flag);
  void close();

  void sendFrame(char *p_data, const size_t data_size);
  void receiveFrame(char *p_data, const size_t data_size);
};

#endif