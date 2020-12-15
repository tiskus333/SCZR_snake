#include <iostream>
#include <vector>
#include <math.h>
#include <ctime>
#include <cstdlib>

#define INIT_LENGTH 50

using namespace std;

int orientation(const pair<int, int> &a, const pair<int, int> &b, const pair<int, int> &c)
{
    int value = int(((b.second - a.second) * (c.first - b.first)) - ((b.first - a.first) * (c.second - b.second)));
    return value == 0 ? 0 : value > 0 ? 1 : 2;
    // if (value == 0)
    //     return 0;
    // else if (value > 0)
    //     return 1;
    // else
    //     return 2;
}

bool ifIntersected(const pair<int, int> &a, const pair<int, int> &b, const pair<int, int> &c, const pair<int, int> &d)
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
    //Snake();
    // void addPoint(const int &x, const int &y)
    // {
    //     snakeBody.push_back(make_pair(x, y));
    // }

    vector<pair<int, int>> snakeBody;
    vector<int> sectionLength;

    int allowedLength = INIT_LENGTH;
    int length = 0;

private:
};

class Fruit
{
public:
    int xFruit, yFruit;

    void generatePoints()
    {
        xFruit = rand() % 200;
        yFruit = rand() % 200;
    }

    bool checkIfEaten(pair<int, int> addedPoint)
    {
        return addedPoint.first > xFruit && addedPoint.first < xFruit + 10 && addedPoint.second > yFruit && addedPoint.second < yFruit + 10 ? true : false;
    }

private:
};

main(int argc, char *argv[])
{
    srand((unsigned)time(0));

    Snake gameSnake;
    gameSnake.snakeBody.push_back(make_pair(5, 2));

    //Read data from Tisku in format int x, y
    //or as a pair

    //example: two variables
    int x, y;
    vector<pair<int, int>> testowy;

    testowy.push_back(make_pair(5, 5));
    testowy.push_back(make_pair(5, 10));
    testowy.push_back(make_pair(3, 7));
    testowy.push_back(make_pair(10, 7));
    testowy.push_back(make_pair(12, 8));
    testowy.push_back(make_pair(10, 10));
    testowy.push_back(make_pair(7, 12));
    testowy.push_back(make_pair(5, 11));

    for (int j = 0; j < testowy.size(); ++j)
    {

        // while (1)
        // {

        x = testowy[j].first;
        y = testowy[j].second;

        cout << "X: " << x << " and Y: " << y << endl;

        int distance = int(sqrt(pow(gameSnake.snakeBody.back().first - x, 2) + pow(gameSnake.snakeBody.back().second - y, 2)));
        cout << "Distance is equal: " << distance << endl;

        if (x != 0 && y != 0) //&& distance > 5)
        {
            gameSnake.snakeBody.push_back(make_pair(x, y));
            gameSnake.sectionLength.push_back(distance);
            gameSnake.length += distance;
            cout << "Lenght of snake: " << gameSnake.length << endl;
        }

        while (gameSnake.length > gameSnake.allowedLength)
        {
            gameSnake.length -= gameSnake.sectionLength.front();
            gameSnake.sectionLength.erase(gameSnake.sectionLength.begin());
            gameSnake.snakeBody.erase(gameSnake.snakeBody.begin());
        }

        //TODO check if he eats fruit
        if (gameSnake.snakeBody.size() > 3)
        {
            cout << "Check if intersected " << endl;
            bool isDead = false;

            pair<int, int> pointA, pointB, pointC, pointD;
            int snakeSize = gameSnake.snakeBody.size();

            pointA = gameSnake.snakeBody[snakeSize - 1];
            pointB = gameSnake.snakeBody[snakeSize - 2];

            for (int i = 0; i < snakeSize - 3; ++i)
            {
                pointC = gameSnake.snakeBody[i];
                pointD = gameSnake.snakeBody[i + 1];

                cout << "Points: "
                     << pointA.first << " " << pointA.second << endl
                     << pointB.first << " " << pointB.second << endl
                     << pointC.first << " " << pointC.second << endl
                     << pointD.first << " " << pointD.second << endl
                     << endl;

                if (ifIntersected(pointA, pointB, pointC, pointD) && pointD != pointB)
                {
                    isDead = true;
                    break;
                }
            }
            if (isDead)
                break;
        }
        // }
    }
}