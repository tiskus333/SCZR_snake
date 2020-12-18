#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string.h>
#include "Snake.hpp"

#pragma once

class SnakeGame
{
private:
    cv::VideoCapture cap;
    Snake snake;
    cv::Mat game_frame, frame, frame_HSV, frame_threshold, color_contour;
    std::string window_game_name;
    std::string window_detection_name;
    
    bool is_paused = false;
    bool configure_options = false;
    bool end_game = false;
    bool repeat_game = false;
    int GAME_SIZE_X = 640;
    int GAME_SIZE_Y = 480;

    // int max_value_H;
    // int max_value;

    // int low_H , low_S , low_V;
    // int high_H , high_S , high_V;
    int max_value_H = 180;
    int max_value = 255;
    int low_H = 32, low_S = 100, low_V = 0;
    int high_H = 100, high_S = max_value, high_V = max_value;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<std::pair<cv::Point2f,float>> circles;
    std::vector<std::vector<cv::Point>> contours_poly;

    char keyHandeler();
    bool getNewFrame();

    static void on_low_H_thresh_trackbar(int, void *);
    static void on_high_H_thresh_trackbar(int, void *);
    static void on_low_S_thresh_trackbar(int, void *);
    static void on_high_S_thresh_trackbar(int, void *);
    static void on_low_V_thresh_trackbar(int, void *);
    static void on_high_V_thresh_trackbar(int, void *);

    
public:
    SnakeGame();
    ~SnakeGame();
    void processImage();
    void drawOptionsWindow();
    void drawGameWindow();
    bool drawStartWindow();
    bool drawEndWindow();
    bool isRepeat();
    bool isEndGame();
};
SnakeGame::SnakeGame(): cap(0), snake({GAME_SIZE_X,GAME_SIZE_Y})
{
    srand((unsigned)time(0));
    if(cap.isOpened()) CV_Assert("Cam opened failed");  // tmp move to proper process
    cap.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);     //
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,GAME_SIZE_Y);     //
    cv::namedWindow(window_game_name);
    //cv::namedWindow(window_detection_name);

    max_value_H = 180;
    max_value = 255;
    low_H = 32; low_S = 100; low_V = 0;
    high_H = 100; high_S = max_value; high_V = max_value;
    window_game_name = "Snake Game";
    window_detection_name = "Object Detection";
}

SnakeGame::~SnakeGame()
{
    cap.release();
}

bool SnakeGame::isRepeat()
{
    return repeat_game;
}

bool SnakeGame::isEndGame()
{
    return end_game;
}

void SnakeGame::processImage()
{
    getNewFrame();
    frame.copyTo(game_frame);

    cv::medianBlur(frame,frame,15);
    // Convert from BGR to HSV colorspace
    cv::cvtColor(frame, frame_HSV, cv::COLOR_BGR2HSV);
    // Detect the object based on HSV Range Values
    cv::inRange(frame_HSV, cv::Scalar(low_H, low_S, low_V), cv::Scalar(high_H, high_S, high_V), frame_threshold);

    cv::morphologyEx(frame_threshold,frame_threshold,cv::MORPH_OPEN,cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(10,10)));
    cv::morphologyEx(frame_threshold,frame_threshold,cv::MORPH_CLOSE,cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(10,10)));
    cv::findContours(frame_threshold, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    contours_poly.clear();
    contours_poly.reserve(contours.size());
    circles.clear();
    circles.reserve(contours.size()); 
    try
    {
        for(size_t i = 0; i < contours.size(); ++i)
        {
            cv::approxPolyDP(contours[i],contours_poly[i],3,true);
            cv::minEnclosingCircle(contours[i],circles[i].first,circles[i].second);
        }
    }
    catch(const cv::Exception& e)
    {
        std::cout<<e.what();
    }
}

void SnakeGame::drawGameWindow()
{
    if(circles.size() > 0 )
    {
        std::sort(circles.begin(), circles.end(),[](const auto &x, const auto &y){return y.second < x.second;});

        cv::circle(game_frame, circles[0].first,circles[0].second,{255,0,0});
        cv::circle(game_frame, circles[0].first,1,{0,0,255});
        if(!is_paused)
        end_game = snake.calculateSnake(circles[0].first);
    }
    snake.draw(game_frame);
    cv::imshow(window_game_name,game_frame);

}

bool SnakeGame::drawStartWindow()
{
    getNewFrame();
    cv::putText(frame,"Press SPACE to begin",{GAME_SIZE_X/2-150, GAME_SIZE_Y/2-20},cv::HersheyFonts::FONT_HERSHEY_DUPLEX,1,{0,0,255},2);
    cv::imshow(window_game_name,frame);
    return cv::waitKey(1) == ' ';
}
bool SnakeGame::drawEndWindow()
{
    getNewFrame();
    cv::putText(frame,"Press SPACE to reset",{GAME_SIZE_X/2-150, GAME_SIZE_Y/2-20},cv::HersheyFonts::FONT_HERSHEY_DUPLEX,1,{0,0,255},2);
    cv::imshow(window_game_name,frame);
    return cv::waitKey(1) == ' ';
}

void SnakeGame::drawOptionsWindow()
{
    if(configure_options)
    {
        getNewFrame();
        cv::createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar,this);
        cv::createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar,this);
        cv::createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar,this);
        cv::createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar,this);
        cv::createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar,this);
        cv::createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar,this);
        cv::imshow(window_detection_name, frame_threshold); // tmp move to proper process
    }
}

char SnakeGame::keyHandeler()
{
    char key = cv::waitKey(1);

    if(key == 27) 
    {
        end_game = true;
        repeat_game = false;
    }
    if(key == 'o') 
    {
        configure_options = !configure_options;
        is_paused = !is_paused;
        if(cv::getWindowProperty(window_detection_name,cv::WINDOW_AUTOSIZE) != -1)
            cv::destroyWindow(window_detection_name);
    }
    if(key == ' ')
    {
        end_game = false;
        repeat_game = true;
    }
    return key;
}
bool SnakeGame::getNewFrame(){
    cap>>frame;
    cv::flip(frame,frame,1);
    return frame.ptr() != nullptr;
}



void SnakeGame::on_low_H_thresh_trackbar(int, void *)
{
    low_H = std::min(high_H-1, low_H);
    cv::setTrackbarPos("Low H", window_detection_name, low_H);
}
void SnakeGame::on_high_H_thresh_trackbar(int, void *)
{
    high_H = std::max(high_H, low_H+1);
    cv::setTrackbarPos("High H", window_detection_name, high_H);
}
void SnakeGame::on_low_S_thresh_trackbar(int, void *)
{
    low_S = std::min(high_S-1, low_S);
    cv::setTrackbarPos("Low S", window_detection_name, low_S);
}
void SnakeGame::on_high_S_thresh_trackbar(int, void *)
{
    high_S = std::max(high_S, low_S+1);
    cv::setTrackbarPos("High S", window_detection_name, high_S);
}
void SnakeGame::on_low_V_thresh_trackbar(int, void *)
{
    low_V = std::min(high_V-1, low_V);
    cv::setTrackbarPos("Low V", window_detection_name, low_V);
}
void SnakeGame::on_high_V_thresh_trackbar(int, void *)
{
    high_V = std::max(high_V, low_V+1);
    cv::setTrackbarPos("High V", window_detection_name, high_V);
}
