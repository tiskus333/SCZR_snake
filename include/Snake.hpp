#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include "Fruit.hpp"

class Snake
{
public:
    std::vector<cv::Point> snakeBody;
    std::vector<int> sectionLength;
    Fruit snakeFruit;

    int allowedLength = 50;
    int length = 0;

Snake()
    {
        snakeFruit.generatePoint();
    }

    void incAllowedLength()
    {
        allowedLength += 5;
    }

    bool ifIntersected(const cv::Point &a, const cv::Point &b, const cv::Point &c, const cv::Point &d)
    {
        int checkOrient[4] = {};
        checkOrient[0] = orientation(a, b, c);
        checkOrient[1] = orientation(a, b, d);
        checkOrient[2] = orientation(c, d, a);
        checkOrient[3] = orientation(c, d, b);
        return (checkOrient[0] != checkOrient[1] && checkOrient[2] != checkOrient[3]) ? true : false;
    }

    int orientation(const cv::Point &a, const cv::Point &b, const cv::Point &c)
    {
        int value = int(((b.y - a.y) * (c.x - b.x)) - ((b.x - a.x) * (c.y - b.y)));
        return value == 0 ? 0 : value > 0 ? 1 : 2;
    }

    void addPointToSnake(const int &x, const int &y)
    {
        snakeBody.emplace_back(x, y);
    }

    void addToSnake(const cv::Point &point)
    {
        snakeBody.push_back(point);
    }

    void addValueToLength(const int &temp)
    {
        sectionLength.emplace_back(temp);
        length += temp;
    }

    void deletePointFromBegin()
    {
        deleteValueFromBegin();
        snakeBody.erase(snakeBody.begin());
    }

    void deleteValueFromBegin()
    {
        length -= sectionLength.front();
        sectionLength.erase(sectionLength.begin());
    }

    cv::Point getPoint(int i)
    {
        return snakeBody[i];
    }

    bool checkIfEaten()
    {
        return snakeFruit.checkIfEat(snakeBody.back());
    }
    void draw(cv::Mat &frame)
    {
        if(snakeBody.size() > 0)
        for( auto it = snakeBody.rbegin(); it != snakeBody.rend() - 1; ++it)
        {
            cv::line(frame,*it, *(it+1),{255,8,127},5);
        }
        cv::circle(frame, snakeFruit.fruitPoint, snakeFruit.fruitRadius, {0,255,0}, cv::FILLED);
    }
};
