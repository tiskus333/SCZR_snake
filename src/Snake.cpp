#include "Snake.hpp"

Snake::Snake(const cv::Point &screenSize) : ScreenSize(screenSize) {
  snakeFruit.generatePoint(ScreenSize);
}

void Snake::incAllowedLength() { allowedLength += ADD_LENGTH; }

bool Snake::ifIntersected(const cv::Point &a, const cv::Point &b,
                          const cv::Point &c, const cv::Point &d) {
  int checkOrient[4] = {};
  checkOrient[0] = orientation(a, b, c);
  checkOrient[1] = orientation(a, b, d);
  checkOrient[2] = orientation(c, d, a);
  checkOrient[3] = orientation(c, d, b);
  return (checkOrient[0] != checkOrient[1]) &&
         (checkOrient[2] != checkOrient[3]);
}

int Snake::orientation(const cv::Point &a, const cv::Point &b,
                       const cv::Point &c) {
  int value = int(((b.y - a.y) * (c.x - b.x)) - ((b.x - a.x) * (c.y - b.y)));
  return value == 0 ? 0 : value > 0 ? 1 : 2;
}

void Snake::addPointToSnake(const int &x, const int &y) {
  snakeBody.emplace_back(x, y);
}

void Snake::addToSnake(const cv::Point &point) { snakeBody.push_back(point); }

void Snake::addValueToLength(const int &temp) {
  sectionLength.emplace_back(temp);
  length += temp;
}

void Snake::deletePointFromBegin() {
  deleteValueFromBegin();
  snakeBody.pop_front();
}

void Snake::deleteValueFromBegin() {
  length -= sectionLength.front();
  sectionLength.pop_front();
}

cv::Point Snake::getPoint(int i) { return snakeBody[i]; }

bool Snake::checkIfEaten() {
  int dist = sectionLength.back() / snakeFruit.fruitRadius + 1;
  int diffX = 0, diffY = 0;
  if (snakeBody.size() > 1) {
    diffX =
        (getPoint(snakeBody.size() - 1).x - getPoint(snakeBody.size() - 2).x) /
        dist;
    diffY =
        (getPoint(snakeBody.size() - 1).y - getPoint(snakeBody.size() - 2).y) /
        dist;
  }

  for (int i = 1; i <= dist; ++i)
    if (snakeFruit.checkIfEat(snakeBody.back() -
                              cv::Point(diffX * i, diffY * i)))
      return true;
  return false;
}

void Snake::draw(cv::Mat &frame) {
  if (snakeBody.size() > 1)
    for (auto it = snakeBody.crbegin(); it != snakeBody.crend() - 1; ++it) {
      cv::line(frame, *it, *(it + 1), {127, 8, 255}, 5);
    }
  cv::circle(frame, snakeFruit.fruitPoint, snakeFruit.fruitRadius, {0, 255, 0},
             cv::FILLED);
  cv::putText(frame, "SCORE: " + std::to_string(score),
              {ScreenSize.x - 200, 30}, cv::HersheyFonts::FONT_HERSHEY_DUPLEX,
              1, {255, 0, 0}, 2);
  cv::putText(frame, "LIVES: " + std::to_string(lives), {20, 30},
              cv::HersheyFonts::FONT_HERSHEY_DUPLEX, 1, {255, 0, 0}, 2);
}

bool Snake::calculateSnake(const cv::Point &point) {
  if (snakeBody.size() < 1) {
    addToSnake(point);
  } else {
    int distance = int(sqrt(pow(snakeBody.back().x - point.x, 2) +
                            pow(snakeBody.back().y - point.y, 2)));

    if (point.x != 0 && point.y != 0) // uncomment later
    {
      addToSnake(point);
      addValueToLength(distance);
    }

    while (length > allowedLength) {
      deletePointFromBegin();
    }

    if (snakeBody.size() > 3) {
      cv::Point pointA, pointB, pointC, pointD;
      int snakeSize = snakeBody.size();

      pointA = getPoint(snakeSize - 1);
      pointB = getPoint(snakeSize - 2);

      for (int i = 0; i < snakeSize - 3; ++i) {
        pointC = getPoint(i);
        pointD = getPoint(i + 1);

        if (ifIntersected(pointA, pointB, pointC, pointD) && pointD != pointB) {
          return --lives == 0;
        }
      }
    }

    if (checkIfEaten()) {
      ++score;
      incAllowedLength();
      snakeFruit.generatePoint(ScreenSize);
    }
  }
  return false;
}