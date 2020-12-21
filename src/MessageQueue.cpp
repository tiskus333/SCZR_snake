#include "MessageQueue.h"
#include <iostream>
MessageQueue::MessageQueue() {
  attributes_.mq_flags = 0;
  attributes_.mq_maxmsg = MSGQ_MAX_MSG;
  attributes_.mq_msgsize = PAYLOAD;
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
  for (; i < data_size / PAYLOAD; ++i) {
    if (mq_send(msgq_des_, p_data + i * PAYLOAD_SIZE, PAYLOAD_SIZE, 1) == -1) {
      perror("mq_send");
      exit(1);
    }
  }
}

void MessageQueue::receiveFrame(char *p_data, const size_t data_size) {
  size_t i = 0;
  uint priority;
  for (; i < data_size / PAYLOAD; ++i) {
    if (mq_receive(msgq_des_, p_data + i * PAYLOAD_SIZE, PAYLOAD_SIZE, &priority) == -1) {
      perror("mq_receive");
      exit(1);
    }
  }
}

void createMessageQueue(const char *msgq_file_name) {
  struct mq_attr frame;
  frame.mq_flags = 0;
  frame.mq_maxmsg = MSGQ_MAX_MSG;
  frame.mq_msgsize = PAYLOAD;
  frame.mq_curmsgs = 0;
  mqd_t msg_q = mq_open(msgq_file_name, O_CREAT | O_EXCL | O_RDWR,
                        S_IREAD | S_IWRITE, &frame);
  if (msg_q == -1) {
    perror("Cannot create message queue");
    exit(EXIT_FAILURE);
  }
}

mqd_t openMessageQueue(const char *msgq_file_name, const int flag) {
  mqd_t msgq_des = mq_open(msgq_file_name, flag);
  if (msgq_des == -1) {
    perror("Cannot open message queue");
    exit(1);
  }
  return msgq_des;
}

void msgqSendFrame(int msgq_des, char *p_data, const size_t data_size) {
  size_t i = 0;
  for (; i < data_size / PAYLOAD; ++i) {
    if (mq_send(msgq_des, p_data + i * PAYLOAD, PAYLOAD, 1) == -1) {
      perror("mq_send");
      exit(1);
    }
  }
}

void msgqReceiveFrame(int msgq_des, char *p_data, const size_t data_size) {
  size_t i = 0;
  uint priority;
  for (; i < data_size / PAYLOAD; ++i) {
    if (mq_receive(msgq_des, p_data + i * PAYLOAD, PAYLOAD, &priority) == -1) {
      perror("mq_receive");
      exit(1);
    }
  }
}
