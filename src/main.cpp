#include "SharedMemory.hpp"
#include "Snake.hpp"
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
int low_H = 32, low_S = 100, low_V = 0;
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

const char *FRAME = "/frame_buffer";
const char *GAME = "/game_frame_buffer";
const char *GAME_STATE = "/game_state";
enum class MEMORY_MODES { SHARED_MEMORY, PIPE, MESSEGE_QUEUE };

void initProcess(void (*fun)()) {
  if (fork() == 0) {
    fun();
    exit(EXIT_SUCCESS);
  }
}

// odczyt kamery: klatka -> pamiec wspoldzielona
void processA() {
  sh_m *shmp = openSharedMemory(FRAME);
  gm_st *game_state = openSharedGameState(GAME_STATE);

  cv::Mat frame;
  cv::VideoCapture camera(0);
  if (camera.isOpened())
    CV_Assert("Cam opened failed");
  camera.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, GAME_SIZE_Y);

  while (game_state->readKey() != 27) {
    camera >> frame;
    shmp->writeFrame(frame);
  }
  // clear after process is finished
  shm_unlink(FRAME);
  shm_unlink(GAME_STATE);
  camera.release();
}

void processB() {
  sh_m *shmp_f = openSharedMemory(FRAME);
  sh_m *shmp_g = openSharedMemory(GAME);
  gm_st *game_state = openSharedGameState(GAME_STATE);

  uchar frame_data[DATA_SIZE];
  cv::Mat frame(GAME_SIZE_Y, GAME_SIZE_X, CV_8UC3, frame_data);

  unsigned char key;

  cv::Mat game_frame, frame_threshold;

  std::vector<std::vector<cv::Point>> contours;

  do {
    Snake snake({GAME_SIZE_X, GAME_SIZE_Y});
    do {
      shmp_f->readFrame(frame);
      cv::flip(frame, frame, 1);
      cv::putText(frame, "Press SPACE to begin",
                  {GAME_SIZE_X / 2 - 150, GAME_SIZE_Y / 2 - 20},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      shmp_g->writeFrame(frame);
      key = game_state->readKey();
      if (key == 27)
        return;
    } while (key != ' ');

    do {
      // cap >> frame;
      shmp_f->readFrame(frame);
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

      shmp_g->writeFrame(game_frame);

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
      }

      key = game_state->readKey();
      if (key == 27) {
        return;
      }
      if (key == 'o') {
        configure_options = !configure_options;
        is_paused = !is_paused;
        if (cv::getWindowProperty(window_detection_name, cv::WINDOW_AUTOSIZE) !=
            -1)
          cv::destroyWindow(window_detection_name);
      }
    } while (!end_game);

    do {
      shmp_f->readFrame(frame);
      cv::flip(frame, frame, 1);
      cv::putText(frame, "GAME OVER!",
                  {GAME_SIZE_X / 2 - 75, GAME_SIZE_Y / 2 - 50},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      cv::putText(frame, "Press SPACE to reset",
                  {GAME_SIZE_X / 2 - 150, GAME_SIZE_Y / 2 - 20},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      shmp_g->writeFrame(frame);

      key = game_state->readKey();
      if (key == ' ')
        repeat_game = true;
      if (key == 27)
        return;
    } while (key != ' ');

  } while (repeat_game);
  shm_unlink(FRAME);
  shm_unlink(GAME);
  shm_unlink(GAME_STATE);
}

void processC() {
  sh_m *shmp = openSharedMemory(GAME);
  gm_st *game_state = openSharedGameState(GAME_STATE);

  uchar frame_data[DATA_SIZE];
  uchar key_pressed = '_';
  cv::Mat frame(GAME_SIZE_Y, GAME_SIZE_X, CV_8UC3, frame_data);
  cv::namedWindow(window_game_name);

  while (key_pressed != 27) {
    shmp->readFrame(frame);
    cv::imshow(window_game_name, frame);
    key_pressed = cv::waitKey(1);
    game_state->writeKey(key_pressed);
    // shmp->writeKey(key_pressed);
  }
  // clear after process is finished
  shm_unlink(GAME);
  shm_unlink(GAME_STATE);
}

int main(int argc, char *argv[]) {
  // unlink from shared memory object if still exists
  MEMORY_MODES mode = MEMORY_MODES::SHARED_MEMORY;
  if (argc == 3) {
    const std::string val = argv[2];
    if (val == "pipe")
      mode = MEMORY_MODES::PIPE;
    if (val == "msgq")
      mode = MEMORY_MODES::MESSEGE_QUEUE;
  }

  shm_unlink(FRAME);
  shm_unlink(GAME);
  shm_unlink(GAME_STATE);

  createSharedMemory(FRAME);
  createSharedMemory(GAME);
  createSharedGameState(GAME_STATE);

  initProcess(processA);
  initProcess(processB);
  initProcess(processC);
  // unlink from shared memory object
  // sleep(35);
  shm_unlink(FRAME);
  shm_unlink(GAME);
  shm_unlink(GAME_STATE);
  exit(EXIT_SUCCESS);
}
