#include "MessageQueue.h"
#include <time.h>

void msgqSendFrame(int msgq_des, char *p_data, const size_t data_size) {
  int i = 0;
  for (i; i < data_size / PAYLOAD; ++i) {
    if (mq_send(msgq_des, p_data + i * PAYLOAD, PAYLOAD, 1) == -1) {
      perror("mq_send");
      exit(1);
    }
  }
}

void msgqReceiveFrame(int msgq_des, char *p_data, const size_t data_size) {
  int i = 0;
  uint priority;
  for (i; i < data_size / PAYLOAD; ++i) {
    if (mq_receive(msgq_des, p_data + i * PAYLOAD, PAYLOAD, &priority) == -1) {
      perror("mq_receive");
      exit(1);
    }
  }
}
