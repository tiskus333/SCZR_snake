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

class MessageQueue {
  mqd_t msgq_des_;
  struct mq_attr attributes_;

  // constants
  const long PAYLOAD_SIZE = 4096;
  const long MSGQ_MAX_MSG = 10;       // maximum 10

  public:
  MessageQueue();
  void create(const char *msgq_file_name);
  void open(const char *msgq_file_name, const int flag);
  void close();

  void sendFrame(char *p_data, const size_t data_size);
  void receiveFrame(char *p_data, const size_t data_size);
};

void createMessageQueue(const char *msgq_file_name);
mqd_t openMessageQueue(const char *msgq_file_name, const int flag);

void msgqSendFrame(int msgq_des, char *p_data, const size_t data_size);
void msgqReceiveFrame(int msgq_des, char *p_data, const size_t data_size);

#endif