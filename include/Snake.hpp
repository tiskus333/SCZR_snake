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
    const int ADD_LENGTH = 5;
    int length = 0;
    int score = 0;
    cv::Point ScreenSize;

Snake(const cv::Point &screenSize) : ScreenSize(screenSize) 
    {
        snakeFruit.generatePoint(ScreenSize);
    }

    void incAllowedLength()
    {
        allowedLength += ADD_LENGTH;
    }

    bool ifIntersected(const cv::Point &a, const cv::Point &b, const cv::Point &c, const cv::Point &d)
    {
        int checkOrient[4] = {};
        checkOrient[0] = orientation(a, b, c);
        checkOrient[1] = orientation(a, b, d);
        checkOrient[2] = orientation(c, d, a);
        checkOrient[3] = orientation(c, d, b);
        return (checkOrient[0] != checkOrient[1]) && (checkOrient[2] != checkOrient[3]);
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
        return snakeFruit.checkIfEat(/* *(snakeBody.end()-2), */ *(snakeBody.end()-1));
    }

    void draw(cv::Mat &frame)
    {
        if(snakeBody.size() > 0)
        for( auto it = snakeBody.rbegin(); it != snakeBody.rend() - 1; ++it)
        {
            cv::line(frame,*it, *(it+1),{127,8,255},5);
        }
        cv::circle(frame, snakeFruit.fruitPoint, snakeFruit.fruitRadius, {0,255,0}, cv::FILLED);
        cv::putText(frame,"SCORE: "+std::to_string(score),{ScreenSize.x - 200, 30},cv::HersheyFonts::FONT_HERSHEY_DUPLEX,1,{255,0,0},2);
    }
    
    bool calculateSnake(const cv::Point &point)
    {
        if (snakeBody.size() < 1)
        {
            addToSnake(point);
        }
        else
        {
            int distance = int(sqrt(pow(snakeBody.back().x - point.x, 2) + pow(snakeBody.back().y - point.y, 2)));

            if (point.x != 0 && point.y != 0) //&& distance > 5) //uncomment later
            {
                addToSnake(point);
                addValueToLength(distance);
            }

            while (length > allowedLength)
            {
                deletePointFromBegin();
            }

            if (snakeBody.size() > 3)
            {
                cv::Point pointA, pointB, pointC, pointD;
                int snakeSize = snakeBody.size();

                pointA = getPoint(snakeSize - 1);
                pointB = getPoint(snakeSize - 2);

                for (int i = 0; i < snakeSize - 3; ++i)
                {
                    pointC = getPoint(i);
                    pointD = getPoint(i + 1);

                    if (ifIntersected(pointA, pointB, pointC, pointD) && pointD != pointB)
                    {
                        return true;
                    }
                }
            }

            if (checkIfEaten())
            {
                ++score;
                incAllowedLength();
                snakeFruit.generatePoint(ScreenSize);
            }
        }
        return false;
    }
};
