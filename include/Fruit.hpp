#include <opencv2/core/types.hpp>

class Fruit
{
public:
    cv::Point fruitPoint;
    int fruitRadius = 20;

    void generatePoint()
    {
        fruitPoint = cv::Point(rand() % 590 + 25, rand() % 430 + 25);
    }

    bool checkIfEat(const cv::Point &addedPoint)
    {
        return sqrt(pow((addedPoint.x - fruitPoint.x),2) + pow((addedPoint.y - fruitPoint.y),2)) <= fruitRadius;
        //return addedPoint.x > fruitPoint.x && addedPoint.x < fruitPoint.x + fruitRadius && addedPoint.y > fruitPoint.y && addedPoint.y < fruitPoint.y + fruitRadius;
    }
};