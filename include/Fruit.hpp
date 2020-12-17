#include <opencv2/core/types.hpp>

class Fruit
{
public:
    cv::Point fruitPoint;
    const int fruitRadius = 20;
    const int windowOffset = 25 + fruitRadius;

    void generatePoint(const cv::Point &screenSize)
    {
        fruitPoint = cv::Point(rand() % (screenSize.x - 2*windowOffset) + windowOffset, rand() % (screenSize.y - 2*windowOffset) + windowOffset);
    }

    bool checkIfEat(/* const cv::Point &previousPoint,  */const cv::Point &addedPoint)
    {
        //return (int)(std::abs((addedPoint.x - previousPoint.x)*(previousPoint.y - fruitPoint.y) - (previousPoint.x - fruitPoint.x)*(addedPoint.y-previousPoint.y))/sqrt((addedPoint.x-previousPoint.x)*(addedPoint.x-previousPoint.x) + (addedPoint.y-previousPoint.y)*(addedPoint.y-previousPoint.y))) < fruitRadius;
        return sqrt(pow((addedPoint.x - fruitPoint.x),2) + pow((addedPoint.y - fruitPoint.y),2)) <= fruitRadius;
        //return addedPoint.x > fruitPoint.x && addedPoint.x < fruitPoint.x + fruitRadius && addedPoint.y > fruitPoint.y && addedPoint.y < fruitPoint.y + fruitRadius;

    }
};