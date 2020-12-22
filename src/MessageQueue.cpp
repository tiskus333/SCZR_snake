#include "MessageQueue.h"
#include <iostream>
MessageQueue::MessageQueue() {
  attributes_.mq_flags = 0;
  attributes_.mq_maxmsg = MSGQ_MAX_MSG;
  attributes_.mq_msgsize = PAYLOAD_SIZE;
  attributes_.mq_curmsgs = 0;
}

void MessageQueue::create(const char *msgq_file_name) {
  msgq_des_ = mq_open(msgq_file_name, O_CREAT | O_EXCL | O_RDWR,
                        S_IREAD | S_IWRITE, &attributes_);
  if (msgq_des_ == -1) {
    perror("Cannot create message queue");
    exit(EXIT_FAILURE);
  }
}

void MessageQueue::open(const char *msgq_file_name, const int flag) {
  msgq_des_ = mq_open(msgq_file_name, flag);
  if (msgq_des_ == -1) {
    perror("Cannot open message queue");
    exit(1);
  }
}

void MessageQueue::close() {
  mq_close(msgq_des_);
}

void MessageQueue::sendFrame(char *p_data, const size_t data_size) {
  size_t i = 0;

  clock_gettime(CLOCK_REALTIME, &timeout_);
  timeout_.tv_sec += TIMEOUT;

  for (; i < data_size / PAYLOAD_SIZE; ++i) {
    if (mq_timedsend(msgq_des_, p_data + i * PAYLOAD_SIZE, PAYLOAD_SIZE, 1, &timeout_) == -1) {
      perror("mq_send");
      exit(1);
    }
  }
}

void MessageQueue::receiveFrame(char *p_data, const size_t data_size) {
  size_t i = 0;
  uint priority;

  clock_gettime(CLOCK_REALTIME, &timeout_);
  timeout_.tv_sec += TIMEOUT;

  for (; i < data_size / PAYLOAD_SIZE; ++i) {
    if (mq_timedreceive(msgq_des_, p_data + i * PAYLOAD_SIZE, PAYLOAD_SIZE, &priority, &timeout_) == -1) {
      perror("mq_receive");
      exit(1);
    }
  }
}