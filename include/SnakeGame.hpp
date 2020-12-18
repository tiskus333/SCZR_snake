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
    cv::VideoCapture cap{0};
    Snake snake{{GAME_SIZE_X,GAME_SIZE_Y}};
    cv::Mat game_frame={}, frame={}, frame_threshold={};
    std::string window_game_name = "Snake Game";
    std::string window_detection_name = "Object Detection";
    
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


    std::vector<std::pair<cv::Point2f,float>> circles={};

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
    char keyHandler();
};
SnakeGame::SnakeGame()
{
    srand((unsigned)time(0));
    if(cap.isOpened()) CV_Assert("Cam opened failed");  // tmp move to proper process
    cap.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);     //
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,GAME_SIZE_Y);     //
    cv::namedWindow(window_game_name);
    //cv::namedWindow(window_detection_name);

    // max_value_H = 180;
    // max_value = 255;
    // low_H = 32; low_S = 100; low_V = 0;
    // high_H = 100; high_S = max_value; high_V = max_value;
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
    cv::cvtColor(frame, frame_threshold, cv::COLOR_BGR2HSV);
    // Detect the object based on HSV Range Values
    cv::inRange(frame_threshold, cv::Scalar(low_H, low_S, low_V), cv::Scalar(high_H, high_S, high_V), frame_threshold);

    //contours.clear();
    std::vector<std::vector<cv::Point>> contours;
    cv::morphologyEx(frame_threshold,frame_threshold,cv::MORPH_OPEN,cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(10,10)));
    cv::morphologyEx(frame_threshold,frame_threshold,cv::MORPH_CLOSE,cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(10,10)));
    cv::findContours(frame_threshold, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    //std::cout<<"morph \n";
    std::vector<std::vector<cv::Point>> contours_poly(contours.size());
    circles.clear();
    circles.reserve(contours.size());
        //std::cout<<contours.size()<<std::endl;
     
    try
    {
        //std::cout<<"circles begin\n";

        for(size_t i = 0; i < contours.size(); ++i)
        {
            cv::approxPolyDP(contours[i],contours_poly[i],3,true);
            //std::cout<<"approx\n";

            cv::minEnclosingCircle(contours[i],circles[i].first,circles[i].second);
            //std::cout<<"encl circl\n";
        }
        if(circles.size() > 0)
        {
            std::sort(circles.begin(), circles.end(),[](const auto &x, const auto &y){return y.second < x.second;});
        
        if(!is_paused)
            end_game = snake.calculateSnake(circles[0].first);
        }
        //std::cout<<"circles end\n";
    }
    catch(const cv::Exception& e)
    {
        std::cout<<e.what();
    }
}

void SnakeGame::drawGameWindow()
{
    //std::cout<<"draw \n";
    if(circles.size() > 0 )
    {
        cv::circle(game_frame, circles[0].first,circles[0].second,{255,0,0});
        cv::circle(game_frame, circles[0].first,1,{0,0,255});
        
    }
        snake.draw(game_frame);
    cv::imshow(window_game_name,game_frame); // tmp move to proper process

}

bool SnakeGame::drawStartWindow()
{
    getNewFrame();
    cv::putText(frame,"Press SPACE to begin",{GAME_SIZE_X/2-150, GAME_SIZE_Y/2-20},cv::HersheyFonts::FONT_HERSHEY_DUPLEX,1,{0,0,255},2);
    cv::imshow(window_game_name,frame);
    return cv::waitKey(2) != ' ';
}
bool SnakeGame::drawEndWindow()
{
    getNewFrame();
    cv::putText(frame,"Press SPACE to reset",{GAME_SIZE_X/2-150, GAME_SIZE_Y/2-20},cv::HersheyFonts::FONT_HERSHEY_DUPLEX,1,{0,0,255},2);
    cv::imshow(window_game_name,frame);
    return cv::waitKey(2) != ' ';
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
        cv::imshow(window_detection_name, frame_threshold); 
    }
}

char SnakeGame::keyHandler()
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
    if(cap.isOpened())
        cap>>frame;
    else  std::cout<<"cap not opened\n";
    
    cv::flip(frame,frame,1);
    return frame.ptr() != nullptr;
}



void SnakeGame::on_low_H_thresh_trackbar(int, void * ptr)
{
    SnakeGame *game = (SnakeGame*)ptr;
    game->low_H = std::min(game->high_H-1, game->low_H);
    cv::setTrackbarPos("Low H", game->window_detection_name, game->low_H);
}
void SnakeGame::on_high_H_thresh_trackbar(int, void * ptr)
{
    SnakeGame *game = (SnakeGame*)ptr;
    game->high_H = std::max( game->high_H,  game->low_H+1);
    cv::setTrackbarPos("High H", game->window_detection_name, game->high_H);
}
void SnakeGame::on_low_S_thresh_trackbar(int, void * ptr)
{
    SnakeGame *game = (SnakeGame*)ptr;
    game->low_S = std::min( game->high_S-1,  game->low_S);
    cv::setTrackbarPos("Low S",game->window_detection_name, game->low_S);
}
void SnakeGame::on_high_S_thresh_trackbar(int, void * ptr)
{
    SnakeGame *game = (SnakeGame*)ptr;
    game->high_S = std::max( game->high_S,  game->low_S+1);
    cv::setTrackbarPos("High S", game->window_detection_name, game->high_S);
}
void SnakeGame::on_low_V_thresh_trackbar(int, void * ptr)
{
    SnakeGame *game = (SnakeGame*)ptr;
    game->low_V = std::min( game->high_V-1,  game->low_V);
    cv::setTrackbarPos("Low V", game->window_detection_name, game->low_V);
}
void SnakeGame::on_high_V_thresh_trackbar(int, void * ptr)
{
    SnakeGame *game = (SnakeGame*)ptr;
    game->high_V = std::max(game->high_V, game->low_V+1);
    cv::setTrackbarPos("High V", game->window_detection_name, game->high_V);
}
