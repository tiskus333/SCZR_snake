#include <iostream>
#include <vector>
#include <math.h>
#include <ctime>
#include <cstdlib>
#include <opencv2/core/types.hpp>

#define INIT_LENGTH 50

using namespace std;

int orientation(const cv::Point &a, const cv::Point &b, const cv::Point &c)
{
    int value = int(((b.y - a.y) * (c.x - b.x)) - ((b.x - a.x) * (c.y - b.y)));
    return value == 0 ? 0 : value > 0 ? 1 : 2;
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

class Snake
{
public:
    vector<cv::Point> snakeBody;
    vector<int> sectionLength;

    int allowedLength = INIT_LENGTH;
    int length = 0;

    void incAllowedLength()
    {
        allowedLength += 5;
    }
};

class Fruit
{
public:
    cv::Point fruitPoint;

    void generatePoint()
    {
        fruitPoint = cv::Point(rand() % 200, rand() % 200);
    }

    bool checkIfEaten(const cv::Point &addedPoint)
    {
        return addedPoint.x > fruitPoint.x && addedPoint.x < fruitPoint.x + 10 && addedPoint.y > fruitPoint.y && addedPoint.y < fruitPoint.y + 10 ? true : false;
    }
};

main(int argc, char *argv[])
{
    srand((unsigned)time(0));

    Snake gameSnake;
    gameSnake.snakeBody.push_back(cv::Point(5, 2));

    Fruit gameFruit;
    gameFruit.generatePoint();

    //Read data from Tisku in format int x, y or as cv::Point

    //example: two variables
    int x, y;
    vector<cv::Point> testowy;

    testowy.push_back(cv::Point(5, 5));
    testowy.push_back(cv::Point(5, 10));
    testowy.push_back(cv::Point(3, 7));
    testowy.push_back(cv::Point(10, 7));
    testowy.push_back(cv::Point(12, 8));
    testowy.push_back(cv::Point(10, 10));
    testowy.push_back(cv::Point(7, 12));
    testowy.push_back(cv::Point(5, 11));

    for (int j = 0; j < testowy.size(); ++j)
    {
        x = testowy[j].x;
        y = testowy[j].y;

        if (gameSnake.snakeBody.size() < 1)
        {
            gameSnake.snakeBody.push_back(cv::Point(x, y));
        }
        else
        {

            int distance = int(sqrt(pow(gameSnake.snakeBody.back().x - x, 2) + pow(gameSnake.snakeBody.back().y - y, 2)));

            if (x != 0 && y != 0) //&& distance > 5) //uncomment later
            {
                gameSnake.snakeBody.push_back(cv::Point(x, y));
                gameSnake.sectionLength.push_back(distance);
                gameSnake.length += distance;
            }

            while (gameSnake.length > gameSnake.allowedLength)
            {
                gameSnake.length -= gameSnake.sectionLength.front();
                gameSnake.sectionLength.erase(gameSnake.sectionLength.begin());
                gameSnake.snakeBody.erase(gameSnake.snakeBody.begin());
            }

            if (gameSnake.snakeBody.size() > 3)
            {
                bool isDead = false;

                cv::Point pointA, pointB, pointC, pointD;
                int snakeSize = gameSnake.snakeBody.size();

                pointA = gameSnake.snakeBody[snakeSize - 1];
                pointB = gameSnake.snakeBody[snakeSize - 2];

                for (int i = 0; i < snakeSize - 3; ++i)
                {
                    pointC = gameSnake.snakeBody[i];
                    pointD = gameSnake.snakeBody[i + 1];

                    if (ifIntersected(pointA, pointB, pointC, pointD) && pointD != pointB)
                    {
                        isDead = true;
                        break;
                    }
                }
                if (isDead)
                    break;
            }

            if (gameFruit.checkIfEaten(gameSnake.snakeBody.back()))
            {
                gameSnake.incAllowedLength();
                gameFruit.generatePoint();
            }
        }
    }
}