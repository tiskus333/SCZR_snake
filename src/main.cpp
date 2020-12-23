#include "MessageQueue.h"
#include "Pipe.h"
#include "SharedMemory.hpp"
#include "Snake.hpp"

#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <string.h>

bool is_paused = false;
bool repeat_game = false;
bool configure_options = false;
bool end_game = false;
int GAME_SIZE_X = 640;
int GAME_SIZE_Y = 480;

const int max_value_H = 180;
const int max_value = 255;
const std::string window_game_name = "Snake Game";
const std::string window_detection_name = "Object Detection";
int low_H = 32, low_S = 100, low_V = 5;
int high_H = 100, high_S = max_value, high_V = max_value;

static void on_low_H_thresh_trackbar(int, void *) {
  low_H = std::min(high_H - 1, low_H);
  cv::setTrackbarPos("Low H", window_detection_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *) {
  high_H = std::max(high_H, low_H + 1);
  cv::setTrackbarPos("High H", window_detection_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *) {
  low_S = std::min(high_S - 1, low_S);
  cv::setTrackbarPos("Low S", window_detection_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *) {
  high_S = std::max(high_S, low_S + 1);
  cv::setTrackbarPos("High S", window_detection_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *) {
  low_V = std::min(high_V - 1, low_V);
  cv::setTrackbarPos("Low V", window_detection_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *) {
  high_V = std::max(high_V, low_V + 1);
  cv::setTrackbarPos("High V", window_detection_name, high_V);
}

// shared memory
const char *FRAME = "/frame_buffer";
const char *GAME = "/game_frame_buffer";
// pipe
int pipe_frame[2], pipe_game[2];
// message queue
const char *MSGQ_FRAME = "/msgq_frame";
const char *MSGQ_GAME = "/msgq_game";

// timestamps
const char *TIMER_A = "/timer_a",
           *TIMER_B_IN = "/timer_b_in",
           *TIMER_B_OUT = "/timer_b_out",
           *TIMER_C = "/timer_c";

// game state
const char *GAME_STATE = "/game_state";

enum class MEMORY_MODES { SHARED_MEMORY, PIPE, MESSAGE_QUEUE };
MEMORY_MODES mode = MEMORY_MODES::SHARED_MEMORY;

void initProcess(void (*fun)()) {
  if (fork() == 0) {
    fun();
    exit(EXIT_SUCCESS);
  }
}

// odczyt kamery: klatka -> pamiec wspoldzielona
void processA() {
  std::cout << "A: " << getpid() << std::endl;

  sh_m *shmp;
  MessageQueue msgq_frame;

  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    shmp = openSharedMemory(FRAME);
    break;
  case MEMORY_MODES::PIPE:
    close(pipe_frame[0]);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    msgq_frame.open(MSGQ_FRAME, O_WRONLY);
    break;
  default:
    break;
  }

  // game state
  gm_st *game_state = openSharedGameState(GAME_STATE);
  
  // timestamps
  time_buffer *timer_a = openSharedTimerBuffer(TIMER_A);
  int64_t *buffer = new int64_t[BUFFER_SIZE];
  int size = 0;

  cv::Mat frame;
  cv::VideoCapture camera(0);
  if (camera.isOpened())
    CV_Assert("Cam opened failed");
  camera.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, GAME_SIZE_Y);

  // game state
  while (game_state->readKey() != 27) {
    camera >> frame;

    // timestamps
    buffer[size] = getTimestamp();
    if( ++size == BUFFER_SIZE ) {
      timer_a->writeBuffer(buffer, size);
      size = 0;
    }

    // SWITCH
    switch (mode) {
    case MEMORY_MODES::SHARED_MEMORY:
      shmp->writeFrame(frame);
      break;
    case MEMORY_MODES::PIPE:
      pipeSend<unsigned char>(pipe_frame[1], frame.data, DATA_SIZE);
      break;
    case MEMORY_MODES::MESSAGE_QUEUE:
      msgq_frame.sendFrame((char *)frame.data, DATA_SIZE);
      break;
    default:
      break;
    }
  }
  if(size != 0)
    timer_a->writeBuffer(buffer, size);

  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    shm_unlink(FRAME);
    break;
  case MEMORY_MODES::PIPE:
    close(pipe_frame[1]);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    msgq_frame.close();
    break;
  default:
    break;
  }

  // timestamps
  shm_unlink(TIMER_A);
  // game state
  shm_unlink(GAME_STATE);

  camera.release();
}

void processB() {
  std::cout << "B: " << getpid() << std::endl;
  sh_m *shmp_f, *shmp_g;
  MessageQueue msgq_frame, msgq_game;
  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    shmp_f = openSharedMemory(FRAME);
    shmp_g = openSharedMemory(GAME);
    break;
  case MEMORY_MODES::PIPE:
    close(pipe_frame[1]);
    close(pipe_game[0]);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    msgq_frame.open(MSGQ_FRAME, O_RDONLY);
    msgq_game.open(MSGQ_GAME, O_WRONLY);
    break;
  default:
    break;
  }

  // timestamps
  time_buffer *timer_b_in = openSharedTimerBuffer(TIMER_B_IN),
              *timer_b_out = openSharedTimerBuffer(TIMER_B_OUT);
  int64_t buffer_in[BUFFER_SIZE], buffer_out[BUFFER_SIZE];
  int size_in = 0, size_out = 0;

  // game state
  gm_st *game_state = openSharedGameState(GAME_STATE);

  uchar frame_data[DATA_SIZE];
  cv::Mat frame(GAME_SIZE_Y, GAME_SIZE_X, CV_8UC3, frame_data);
  cv::namedWindow(window_detection_name);

  unsigned char key;

  cv::Mat game_frame, frame_threshold;

  std::vector<std::vector<cv::Point>> contours;

  do {
    Snake snake({GAME_SIZE_X, GAME_SIZE_Y});
    do {
      // SWITCH
      switch (mode) {
      case MEMORY_MODES::SHARED_MEMORY:
        shmp_f->readFrame(frame);
        break;
      case MEMORY_MODES::PIPE:
        pipeReceive<unsigned char>(pipe_frame[0], frame.data, DATA_SIZE);
        break;
      case MEMORY_MODES::MESSAGE_QUEUE:
        msgq_frame.receiveFrame((char *)frame.data, DATA_SIZE);
        break;
      default:
        break;
      }

      // timestamps
      buffer_in[size_in] = getTimestamp();
      if( ++size_in == BUFFER_SIZE ) {
        timer_b_in->writeBuffer(buffer_in, size_in);
        size_in = 0;
      }
      

      cv::flip(frame, frame, 1);
      cv::putText(frame, "Press SPACE to begin",
                  {GAME_SIZE_X / 2 - 150, GAME_SIZE_Y / 2 - 20},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);

      // timestamps
      buffer_out[size_out] = getTimestamp();
      if( ++size_out == BUFFER_SIZE ) {
        timer_b_out->writeBuffer(buffer_out, size_out);
        size_out = 0;
      }

      // SWITCH
      switch (mode) {
      case MEMORY_MODES::SHARED_MEMORY:
        shmp_g->writeFrame(frame);
        break;
      case MEMORY_MODES::PIPE:
        pipeSend<unsigned char>(pipe_game[1], frame.data, DATA_SIZE);
        break;
      case MEMORY_MODES::MESSAGE_QUEUE:
        msgq_game.sendFrame((char *)frame.data, DATA_SIZE);
        break;
      default:
        break;
      }

      // game state
      key = game_state->readKey();
      if (key == 27)
        return;
    } while (key != ' ');

    do {
      // SWITCH
      switch (mode) {
      case MEMORY_MODES::SHARED_MEMORY:
        shmp_f->readFrame(frame);
        break;
      case MEMORY_MODES::PIPE:
        pipeReceive<unsigned char>(pipe_frame[0], frame.data, DATA_SIZE);
        break;
      case MEMORY_MODES::MESSAGE_QUEUE:
        msgq_frame.receiveFrame((char *)frame.data, DATA_SIZE);
        break;
      default:
        break;
      }

      // timestamps
      buffer_in[size_in] = getTimestamp();
      if( ++size_in == BUFFER_SIZE ) {
        timer_b_in->writeBuffer(buffer_in, size_in);
        size_in = 0;
      }

      cv::flip(frame, frame, 1);
      frame.copyTo(game_frame);

      cv::medianBlur(frame, frame, 15);
      // Convert from BGR to HSV colorspace
      cv::cvtColor(frame, frame_threshold, cv::COLOR_BGR2HSV);
      // Detect the object based on HSV Range Values
      cv::inRange(frame_threshold, cv::Scalar(low_H, low_S, low_V),
                  cv::Scalar(high_H, high_S, high_V), frame_threshold);

      cv::morphologyEx(frame_threshold, frame_threshold, cv::MORPH_OPEN,
                       cv::getStructuringElement(cv::MorphShapes::MORPH_RECT,
                                                 cv::Size(10, 10)));
      cv::morphologyEx(frame_threshold, frame_threshold, cv::MORPH_CLOSE,
                       cv::getStructuringElement(cv::MorphShapes::MORPH_RECT,
                                                 cv::Size(10, 10)));
      cv::findContours(frame_threshold, contours, cv::RETR_TREE,
                       cv::CHAIN_APPROX_SIMPLE);

      std::vector<std::vector<cv::Point>> contours_poly(contours.size());
      std::vector<std::pair<cv::Point2f, float>> circles(contours.size());
      try {
        for (size_t i = 0; i < contours.size(); ++i) {
          cv::approxPolyDP(contours[i], contours_poly[i], 3, true);
          cv::minEnclosingCircle(contours[i], circles[i].first,
                                 circles[i].second);
        }
      } catch (const cv::Exception &e) {
        std::cout << e.what();
      }

      if (circles.size() > 0) {
        std::sort(
            circles.begin(), circles.end(),
            [](const auto &x, const auto &y) { return y.second < x.second; });

        cv::circle(game_frame, circles[0].first, circles[0].second,
                   {255, 0, 0});
        cv::circle(game_frame, circles[0].first, 1, {0, 0, 255});
        if (!is_paused)
          end_game = snake.calculateSnake(circles[0].first);
      }
      snake.draw(game_frame);

      if (configure_options) {
        cv::createTrackbar("Low H", window_detection_name, &low_H, max_value_H,
                           on_low_H_thresh_trackbar);
        cv::createTrackbar("High H", window_detection_name, &high_H,
                           max_value_H, on_high_H_thresh_trackbar);
        cv::createTrackbar("Low S", window_detection_name, &low_S, max_value,
                           on_low_S_thresh_trackbar);
        cv::createTrackbar("High S", window_detection_name, &high_S, max_value,
                           on_high_S_thresh_trackbar);
        cv::createTrackbar("Low V", window_detection_name, &low_V, max_value,
                           on_low_V_thresh_trackbar);
        cv::createTrackbar("High V", window_detection_name, &high_V, max_value,
                           on_high_V_thresh_trackbar);
        cv::imshow(window_detection_name, frame_threshold);
        if (cv::waitKey(1) == 'o') {
          is_paused = false;
          configure_options = false;
          cv::destroyWindow(window_detection_name);
        }
      }
      // timestamps
      buffer_out[size_out] = getTimestamp();
      if( ++size_out == BUFFER_SIZE ) {
        timer_b_out->writeBuffer(buffer_out, size_out);
        size_out = 0;
      }

      // SWITCH
      switch (mode) {
      case MEMORY_MODES::SHARED_MEMORY:
        shmp_g->writeFrame(game_frame);
        break;
      case MEMORY_MODES::PIPE:
        pipeSend<unsigned char>(pipe_game[1], game_frame.data, DATA_SIZE);
        break;
      case MEMORY_MODES::MESSAGE_QUEUE:
        msgq_game.sendFrame((char *)game_frame.data, DATA_SIZE);
        break;
      default:
        break;
      }

      // game state
      key = game_state->readKey();
      if (key == 27) {
        return;
      }
      if (key == 'o') {
        configure_options = true;
        is_paused = true;
      }
    } while (!end_game);

    do {
      // SWITCH
      switch (mode) {
      case MEMORY_MODES::SHARED_MEMORY:
        shmp_f->readFrame(frame);
        break;
      case MEMORY_MODES::PIPE:
        pipeReceive<unsigned char>(pipe_frame[0], frame.data, DATA_SIZE);
        break;
      case MEMORY_MODES::MESSAGE_QUEUE:
        msgq_frame.receiveFrame((char *)frame.data, DATA_SIZE);
        break;
      default:
        break;
      }

      // timestamps
      buffer_in[size_in] = getTimestamp();
      if( ++size_in == BUFFER_SIZE ) {
        timer_b_in->writeBuffer(buffer_in, size_in);
        size_in = 0;
      }

      cv::flip(frame, frame, 1);
      cv::putText(frame, "GAME OVER!",
                  {GAME_SIZE_X / 2 - 75, GAME_SIZE_Y / 2 - 50},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      cv::putText(frame, "Press SPACE to reset",
                  {GAME_SIZE_X / 2 - 150, GAME_SIZE_Y / 2 - 20},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);

      // timestamps
      buffer_out[size_out] = getTimestamp();
      if( ++size_out == BUFFER_SIZE ) {
        timer_b_out->writeBuffer(buffer_out, size_out);
        size_out = 0;
      }

      // SWITCH
      switch (mode) {
      case MEMORY_MODES::SHARED_MEMORY:
        shmp_g->writeFrame(frame);
        break;
      case MEMORY_MODES::PIPE:
        pipeSend<unsigned char>(pipe_game[1], frame.data, DATA_SIZE);
        break;
      case MEMORY_MODES::MESSAGE_QUEUE:
        msgq_game.sendFrame((char *)frame.data, DATA_SIZE);
        break;
      default:
        break;
      }

      // game state
      key = game_state->readKey();
      if (key == ' ')
        repeat_game = true;
      if (key == 27) {
        end_game = true;
        repeat_game = false;
        key = ' ';
      }
    } while (key != ' ');

  } while (repeat_game);

  if(size_in != 0)
    timer_b_in->writeBuffer(buffer_in, size_in);
  if(size_out != 0)
    timer_b_out->writeBuffer(buffer_out, size_out);

  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    shm_unlink(FRAME);
    shm_unlink(GAME);
    break;
  case MEMORY_MODES::PIPE:
    close(pipe_frame[0]);
    close(pipe_game[1]);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    msgq_frame.close();
    msgq_game.close();
    break;
  default:
    break;
  }

  // timestamps
  shm_unlink(TIMER_B_IN);
  shm_unlink(TIMER_B_OUT);
  // game state
  shm_unlink(GAME_STATE);
}

void processC() {
  std::cout << "C: " << getpid() << std::endl;

  sh_m *shmp;
  MessageQueue msgq_game;

  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    shmp = openSharedMemory(GAME);
    break;
  case MEMORY_MODES::PIPE:
    close(pipe_game[1]);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    msgq_game.open(MSGQ_GAME, O_RDONLY);
    break;
  default:
    break;
  }

  // timestamps
  time_buffer *timer_c = openSharedTimerBuffer(TIMER_C);
  int64_t buffer[BUFFER_SIZE];
  int size = 0;

  // game state
  gm_st *game_state = openSharedGameState(GAME_STATE);

  uchar frame_data[DATA_SIZE];
  uchar key_pressed = '_';
  cv::Mat frame(GAME_SIZE_Y, GAME_SIZE_X, CV_8UC3, frame_data);
  cv::namedWindow(window_game_name);

  while (key_pressed != 27) {
    // SWITCH
    switch (mode) {
    case MEMORY_MODES::SHARED_MEMORY:
      shmp->readFrame(frame);
      break;
    case MEMORY_MODES::PIPE:
      pipeReceive<unsigned char>(pipe_game[0], frame.data, DATA_SIZE);
      break;
    case MEMORY_MODES::MESSAGE_QUEUE:
      msgq_game.receiveFrame((char *)frame.data, DATA_SIZE);
      break;
    default:
      break;
    }

    // timestamps
    buffer[size] = getTimestamp();
    if( ++size == BUFFER_SIZE ) {
      timer_c->writeBuffer(buffer, size);
      size = 0;
    }

    cv::imshow(window_game_name, frame);
    key_pressed = cv::waitKey(1);

    // game state
    game_state->writeKey(key_pressed);
  }

  if(size != 0)
    timer_c->writeBuffer(buffer, size);

  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    shm_unlink(GAME);
    break;
  case MEMORY_MODES::PIPE:
    close(pipe_game[0]);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    msgq_game.close();
    break;
  default:
    break;
  }

  // timestamps
  shm_unlink(TIMER_C);
  // game state
  shm_unlink(GAME_STATE);
}

// // //

int main(int argc, char *argv[]) {
  std::cout << "M: " << getpid() << std::endl;
  // unlink from shared memory object if still exists
  mode = MEMORY_MODES::SHARED_MEMORY;
  std::string ipc_mode = "SHARED_MEMORY";
  if (argc == 2) {
    const std::string val = argv[1];
    if (val == "pipes") {
      mode = MEMORY_MODES::PIPE;
      ipc_mode = "PIPES";
    }
    if (val == "msgq") {
      mode = MEMORY_MODES::MESSAGE_QUEUE;
      ipc_mode = "MESSAGE_QUEUE";
    }
  }
  std::cout << "USING MODE: " << ipc_mode << std::endl
            << "M: " << getpid() << std::endl;

  std::ofstream output_a("wyniki/" + ipc_mode + "_A.txt");
  std::ofstream output_b_in("wyniki/" + ipc_mode + "_B_IN.txt");
  std::ofstream output_b_out("wyniki/" + ipc_mode + "_B_OUT.txt");
  std::ofstream output_c("wyniki/" + ipc_mode + "_C.txt");

  // shared memory
  shm_unlink(FRAME);
  shm_unlink(GAME);
  // message queue
  mq_unlink(MSGQ_FRAME);
  mq_unlink(MSGQ_GAME);
  // game state
  shm_unlink(GAME_STATE);
  // time buffers
  shm_unlink(TIMER_A);
  shm_unlink(TIMER_B_IN);
  shm_unlink(TIMER_B_OUT);
  shm_unlink(TIMER_C);

  // game state
  gm_st *game_state = createSharedGameState(GAME_STATE);

  // time buffers
  time_buffer *timer_a = createSharedTimerBuffer(TIMER_A);
  time_buffer *timer_b_in = createSharedTimerBuffer(TIMER_B_IN);
  time_buffer *timer_b_out = createSharedTimerBuffer(TIMER_B_OUT);
  time_buffer *timer_c = createSharedTimerBuffer(TIMER_C);

  MessageQueue frame, game;

  switch (mode) {
  case MEMORY_MODES::SHARED_MEMORY:
    createSharedMemory(FRAME);
    createSharedMemory(GAME);
    break;
  case MEMORY_MODES::PIPE:
    createPipe(pipe_frame);
    createPipe(pipe_game);
    break;
  case MEMORY_MODES::MESSAGE_QUEUE:
    frame.create(MSGQ_FRAME);
    game.create(MSGQ_GAME);
    break;
  default:
    break;
  }

  initProcess(processA);
  initProcess(processB);
  initProcess(processC);

  int64_t *buff_a = new int64_t[BUFFER_SIZE], 
          buff_b_in[BUFFER_SIZE],
          buff_b_out[BUFFER_SIZE],
          buff_c[BUFFER_SIZE];

  while (game_state->readKey() != 27) {
    if(timer_a->tryReadBuffer(buff_a)) {
      std::cout << " A" << std::endl;
      for(size_t i = 0; i < timer_a->size_; ++i ) {
        output_a << buff_a[i] << "\n";
      }
      timer_a->size_ = 0;
    }
    if(timer_b_in->tryReadBuffer(buff_b_in)) {
      std::cout << " B IN" << std::endl;
      for(size_t i = 0; i < timer_b_in->size_; ++i ) {
        output_b_in << buff_b_in[i] << "\n";
      }
      timer_b_in->size_ = 0;
    }
    if(timer_b_out->tryReadBuffer(buff_b_out)) {
      std::cout << " B OUT" << std::endl;
      for(size_t i = 0; i < timer_b_out->size_; ++i ) {
        output_b_out << buff_b_out[i] << "\n";
      }
      timer_b_out->size_ = 0;
    }
    if(timer_c->tryReadBuffer(buff_c)) {
      std::cout << " C" << std::endl;
      for(size_t i = 0; i < timer_c->size_; ++i ) {
        output_c << buff_c[i] << "\n";
      }
      timer_c->size_ = 0;
    }
  }

  sleep(3);
  
  if(timer_a->tryReadBuffer(buff_a)) {
    std::cout << " A reszta" << std::endl;
    for(size_t i = 0; i < timer_a->size_; ++i ) {
      output_a << buff_a[i] << "\n";
    }
    timer_a->size_ = 0;
  }
  if(timer_b_in->tryReadBuffer(buff_b_in)) {
    std::cout << " B IN reszta" << std::endl;
    for(size_t i = 0; i < timer_b_in->size_; ++i ) {
      output_b_in << buff_b_in[i] << "\n";
    }
    timer_b_in->size_ = 0;
  }
  if(timer_b_out->tryReadBuffer(buff_b_out)) {
    std::cout << " B OUT reszta" << std::endl;
    for(size_t i = 0; i < timer_b_out->size_; ++i ) {
      output_b_out << buff_b_out[i] << "\n";
    }
    timer_b_out->size_ = 0;
  }
  if(timer_c->tryReadBuffer(buff_c)) {
    std::cout << " C reszta" << std::endl;
    for(size_t i = 0; i < timer_c->size_; ++i ) {
      output_c << buff_c[i] << "\n";
    }
    timer_c->size_ = 0;
  }
  output_a.close();
  output_b_in.close();
  output_b_out.close();
  output_c.close();

  // shared memory
  shm_unlink(FRAME);
  shm_unlink(GAME);
  // message queue
  mq_unlink(MSGQ_FRAME);
  mq_unlink(MSGQ_GAME);

  // time buffers
  shm_unlink(TIMER_A);
  shm_unlink(TIMER_B_IN);
  shm_unlink(TIMER_B_OUT);
  shm_unlink(TIMER_C);

  // game state
  shm_unlink(GAME_STATE);
  exit(EXIT_SUCCESS);
}