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

/* static void on_low_H_thresh_trackbar(int, void *) {
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
} */
/* int main() {
  srand((unsigned)time(0));

  cv::VideoCapture cap(0);
  if (cap.isOpened())
    CV_Assert("Cam opened failed");
  cap.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, GAME_SIZE_Y);
  cv::namedWindow(window_game_name);
  char key;

  cv::Mat game_frame, frame, frame_HSV, frame_threshold, color_contour;

  std::vector<std::vector<cv::Point>> contours;

  do {
    Snake snake({GAME_SIZE_X, GAME_SIZE_Y});
    do {
      cap >> frame;
      cv::flip(frame, frame, 1);
      cv::putText(frame, "Press SPACE to begin",
                  {GAME_SIZE_X / 2 - 150, GAME_SIZE_Y / 2 - 20},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      cv::imshow(window_game_name, frame);
      key = cv::waitKey(1);
      if (key == 27)
        return 0;
    } while (key != ' ');

    do {
      cap >> frame;
      cv::flip(frame, frame, 1);
      frame.copyTo(game_frame);

      cv::medianBlur(frame, frame, 15);
      // Convert from BGR to HSV colorspace
      cv::cvtColor(frame, frame_HSV, cv::COLOR_BGR2HSV);
      // Detect the object based on HSV Range Values
      cv::inRange(frame_HSV, cv::Scalar(low_H, low_S, low_V),
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

      // std::cout<<sizeof(game_frame.elemSize() * game_frame.size().area());
      // get frame from smh
      cv::imshow(window_game_name, game_frame);

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

      key = cv::waitKey(1);
      if (key == 27) {
        cap.release();
        return 0;
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
      cap >> frame;
      cv::flip(frame, frame, 1);
      cv::putText(frame, "GAME OVER!",
                  {GAME_SIZE_X / 2 - 75, GAME_SIZE_Y / 2 - 50},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      cv::putText(frame, "Press SPACE to reset",
                  {GAME_SIZE_X / 2 - 150, GAME_SIZE_Y / 2 - 20},
                  cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {0, 0, 255}, 2);
      cv::imshow(window_game_name, frame);
      key = cv::waitKey(1);
      if (key == ' ')
        repeat_game = true;
      if (key == 27)
        return 0;
    } while (key != ' ');

  } while (repeat_game);
  cap.release();
  return 0;
} */

const char *SHARED_MEMORY_PATH = "/frame_buffer";

void initProcess(void (*fun)()) {
  if (fork() == 0) {
    fun();
    exit(EXIT_SUCCESS);
  }
}

// odczyt kamery: klatka -> pamiec wspoldzielona
void processA() {
  std::cout << getpid() << std::endl;
  sh_m *shmp = openSharedMemory(SHARED_MEMORY_PATH);
  cv::Mat frame;
  cv::VideoCapture camera(0);
  if (camera.isOpened())
    CV_Assert("Cam opened failed");
  camera.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, GAME_SIZE_Y);

  for (int i = 0; i < 1000; ++i) {
    camera >> frame;
    cv::imshow("WRITING", frame);
    cv::waitKey(1);

    // close write semaphore
    if (sem_wait(&shmp->sem_write)) {
      perror("A: sem_wait error.");
      exit(EXIT_FAILURE);
    }
    std::cout << " writing \n";
    // writing data into shared memory
    shmp->sendToSharedMemory(frame.data, DATA_SIZE);

    // open read semaphore
    if (sem_post(&shmp->sem_read)) {
      perror("A: sem_post error.");
      exit(EXIT_FAILURE);
    }
    std::cout << "Leave writing \n";
  }
  // clear after process is finished
  shm_unlink(SHARED_MEMORY_PATH);
  camera.release();
}

void processC() {
  sh_m *shmp = openSharedMemory(SHARED_MEMORY_PATH);
  uchar frame_data[DATA_SIZE];
  cv::Mat frame(GAME_SIZE_Y, GAME_SIZE_X, CV_8UC3, frame_data);
  cv::namedWindow(window_game_name);
  std::cout << getpid() << std::endl;

  for (int i = 0; i < 1000; ++i) {

    // close read semaphore
    if (sem_wait(&shmp->sem_read)) {
      perror("A: sem_wait error.");
      exit(EXIT_FAILURE);
    }
    std::cout << " reading \n";
    // receiving data from shared memory
    shmp->receiveFromSharedMemory(frame_data, DATA_SIZE);

    // open read semaphore
    if (sem_post(&shmp->sem_write)) {
      perror("A: sem_post error.");
      exit(EXIT_FAILURE);
    }
    std::cout << "Leave reading \n";
    // memcpy(frame.data, frame_data, DATA_SIZE);

    cv::imshow(window_game_name, frame);
    cv::waitKey(1);
  }
  // clear after process is finished
  shm_unlink(SHARED_MEMORY_PATH);
}

int main() {
  // unlink from shared memory object if still exists
  shm_unlink(SHARED_MEMORY_PATH);

  createSharedMemory(SHARED_MEMORY_PATH);

  initProcess(processA);
  initProcess(processC);
  // unlink from shared memory object
  sleep(30);
  shm_unlink(SHARED_MEMORY_PATH);
  exit(EXIT_SUCCESS);
}
