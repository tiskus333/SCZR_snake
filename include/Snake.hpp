#include "Fruit.hpp"
#include <cmath>
#include <deque>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#pragma once

class Snake {
public:
  std::deque<cv::Point> snakeBody;
  std::deque<int> sectionLength;
  Fruit snakeFruit;

  int allowedLength = 100;
  const int ADD_LENGTH = 10;
  int length = 0;
  int score = 0;
  int lives = 3;
  cv::Point ScreenSize;

  Snake(const cv::Point &screenSize);
  void incAllowedLength();
  bool ifIntersected(const cv::Point &a, const cv::Point &b, const cv::Point &c,
                     const cv::Point &d);
  int orientation(const cv::Point &a, const cv::Point &b, const cv::Point &c);
  void addPointToSnake(const int &x, const int &y);
  void addToSnake(const cv::Point &point);
  void addValueToLength(const int &temp);
  void deletePointFromBegin();
  void deleteValueFromBegin();
  cv::Point getPoint(int i);
  bool checkIfEaten();
  void draw(cv::Mat &frame);
  bool calculateSnake(const cv::Point &point);
};
