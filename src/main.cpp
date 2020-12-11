#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
using namespace cv;
const int max_value_H = 180;
const int max_value = 255;
const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";
int low_H = 160, low_S = 100, low_V = 100;
int high_H = max_value_H, high_S = max_value, high_V = max_value;
static void on_low_H_thresh_trackbar(int, void *)
{
    low_H = min(high_H-1, low_H);
    setTrackbarPos("Low H", window_detection_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *)
{
    high_H = max(high_H, low_H+1);
    setTrackbarPos("High H", window_detection_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *)
{
    low_S = min(high_S-1, low_S);
    setTrackbarPos("Low S", window_detection_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *)
{
    high_S = max(high_S, low_S+1);
    setTrackbarPos("High S", window_detection_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *)
{
    low_V = min(high_V-1, low_V);
    setTrackbarPos("Low V", window_detection_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *)
{
    high_V = max(high_V, low_V+1);
    setTrackbarPos("High V", window_detection_name, high_V);
}
int main()
{
    VideoCapture cap(0);
    if(cap.isOpened()) CV_Assert("Cam opened failed");
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT,480);
    namedWindow(window_capture_name);
    namedWindow(window_detection_name);
    // Trackbars to set thresholds for HSV values
    createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
    createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
    createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
    createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
    createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
    createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
    Mat frame, frame_HSV, frame_threshold, color_contour;
    std::vector<std::vector<Point>> contours;
    while (true) {
        cap >> frame;
        if(frame.empty())
        {
            break;
        }
        medianBlur(frame,frame,15);
        // Convert from BGR to HSV colorspace
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);
        // Detect the object based on HSV Range Values
        inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold);
        // Show the frames
        flip(frame,frame,1);
        flip(frame_threshold,frame_threshold,1);
        // dilate(frame_threshold, frame_threshold, getStructuringElement(MorphShapes::MORPH_RECT, Size(3,3)));
        // erode(frame_threshold, frame_threshold,getStructuringElement(MorphShapes::MORPH_RECT, Size(3,3))); 


        // erode(frame_threshold, frame_threshold,getStructuringElement(MorphShapes::MORPH_RECT, Size(3,3))); 
        // dilate(frame_threshold, frame_threshold, getStructuringElement(MorphShapes::MORPH_RECT, Size(3,3)));

        morphologyEx(frame_threshold,frame_threshold,MORPH_OPEN,getStructuringElement(MorphShapes::MORPH_RECT, Size(10,10)));
        morphologyEx(frame_threshold,frame_threshold,MORPH_CLOSE,getStructuringElement(MorphShapes::MORPH_RECT, Size(10,10)));
        findContours(frame_threshold, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        for(size_t i =0; i<contours.size();++i)
            drawContours(frame,contours,i,Scalar(255,255,255));
        
        std::vector<std::vector<Point>> contours_poly(contours.size());
        std::vector<std::pair<Point2f,float>> circles(contours.size()); 
        try
        {
            for(size_t i = 0; i < contours.size(); ++i)
            {
                approxPolyDP(contours[i],contours_poly[i],3,true);
                minEnclosingCircle(contours[i],circles[i].first,circles[i].second);
                //auto rect = boundingRect(contours_poly);
                //if(rect.area())
                //rectangle(frame,rect, Scalar(255,0,0),3);
                //if(radiuses[i] > 10)
                //circle(frame,centers[i],radiuses[i],Scalar(255,0,0));
            }
        }
        catch(const Exception& e)
        {
            std::cout<<e.what();
        }
        std::sort(circles.begin(), circles.end(),[](const auto &x, const auto &y){return y.second < x.second;});
        if(circles.size() > 0 )
        {
            circle(frame, circles[0].first,circles[0].second,{255,0,0});
            circle(frame, circles[0].first,1,{0,0,255});
        }
        imshow(window_capture_name, frame);
        imshow(window_detection_name, frame_threshold);
        char key = (char) waitKey(30);
        if (key == 'q' || key == 27)
        {
            break;
        }
    }
    cap.release();
    return 0;
}