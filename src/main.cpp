#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include "Snake.hpp"

#define INIT_LENGTH 5000;

///SHARED
bool is_paused = false;
bool configure_options = false;
bool end_game = false;
int GAME_SIZE_X = 640;
int GAME_SIZE_Y = 480;
///SHARED


const int max_value_H = 180;
const int max_value = 255;
const std::string window_game_name = "Snake Game";
const std::string window_detection_name = "Object Detection";
int low_H = 32, low_S = 100, low_V = 0;
int high_H = 100, high_S = max_value, high_V = max_value;

static void on_low_H_thresh_trackbar(int, void *)
{
    low_H = std::min(high_H-1, low_H);
    cv::setTrackbarPos("Low H", window_detection_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *)
{
    high_H = std::max(high_H, low_H+1);
    cv::setTrackbarPos("High H", window_detection_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *)
{
    low_S = std::min(high_S-1, low_S);
    cv::setTrackbarPos("Low S", window_detection_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *)
{
    high_S = std::max(high_S, low_S+1);
    cv::setTrackbarPos("High S", window_detection_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *)
{
    low_V = std::min(high_V-1, low_V);
    cv::setTrackbarPos("Low V", window_detection_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *)
{
    high_V = std::max(high_V, low_V+1);
    cv::setTrackbarPos("High V", window_detection_name, high_V);
}
int main()
{
    srand((unsigned)time(0));
    Snake snake({GAME_SIZE_X,GAME_SIZE_Y});

    cv::VideoCapture cap(0);
    if(cap.isOpened()) CV_Assert("Cam opened failed");
    cap.set(cv::CAP_PROP_FRAME_WIDTH, GAME_SIZE_X);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,GAME_SIZE_Y);
    cv::namedWindow(window_game_name);
    

    cv::Mat game_frame, frame, frame_HSV, frame_threshold, color_contour;

    std::vector<std::vector<cv::Point>> contours;
    while (true) {
        cap >> frame;
        if(frame.empty())
        {
            break;
        }

        cv::flip(frame,frame,1);
        frame.copyTo(game_frame);
        ///SHARED_MEMORY push(frame);

        cv::medianBlur(frame,frame,15);
        // Convert from BGR to HSV colorspace
        cv::cvtColor(frame, frame_HSV, cv::COLOR_BGR2HSV);
        // Detect the object based on HSV Range Values
        cv::inRange(frame_HSV, cv::Scalar(low_H, low_S, low_V), cv::Scalar(high_H, high_S, high_V), frame_threshold);

        cv::morphologyEx(frame_threshold,frame_threshold,cv::MORPH_OPEN,cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(10,10)));
        cv::morphologyEx(frame_threshold,frame_threshold,cv::MORPH_CLOSE,cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(10,10)));
        cv::findContours(frame_threshold, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        
        std::vector<std::vector<cv::Point>> contours_poly(contours.size());
        std::vector<std::pair<cv::Point2f,float>> circles(contours.size()); 
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

        if(configure_options)
        {
            cv::createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
            cv::createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
            cv::createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
            cv::createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
            cv::createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
            cv::createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
            cv::imshow(window_detection_name, frame_threshold);
        }
        if(end_game)
        std::cout<<"GAME OVER!"<<std::endl;

        
        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27)
        {
            cap.release();
            return 0;
        }
        if(key == 'o') 
        {
            configure_options = !configure_options;
            is_paused = !is_paused;
            if(cv::getWindowProperty(window_detection_name,cv::WINDOW_AUTOSIZE) != -1)
                cv::destroyWindow(window_detection_name);
        }
    }
    cap.release();
    return 0;
}

