#include <opencv2/core/types.hpp>
#pragma once
class Fruit {
public:
  cv::Point fruitPoint;
  const int fruitRadius = 20;
  const int windowOffset = 25 + fruitRadius;

  void generatePoint(const cv::Point &screenSize) {
    fruitPoint =
        cv::Point(rand() % (screenSize.x - 2 * windowOffset) + windowOffset,
                  rand() % (screenSize.y - 2 * windowOffset) + windowOffset);
  }

  bool checkIfEat(const cv::Point &addedPoint) {
    return sqrt(pow((addedPoint.x - fruitPoint.x), 2) +
                pow((addedPoint.y - fruitPoint.y), 2)) <= fruitRadius;
  }
};