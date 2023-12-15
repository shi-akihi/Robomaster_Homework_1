#include <assert.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv4/opencv2/core/base.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/core/types.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <vector>

void using_minAreaRect(cv::Mat &image, cv::Mat &contour_image);
void using_approxPolyDP(cv::Mat &image, cv::Mat &contour_image);
int main()
{
    std::string filename[5] = { "plates/001.jpg", "plates/002.jpg", "plates/003.jpg", "plates/004.jpg", "plates/005.jpg" };
    std::string result_path[5] = { "results/001.jpg", "results/002.jpg", "results/003.jpg", "results/004.jpg", "results/005.jpg"};
    for(int i = 0;i < 5;i++) {
        cv::Mat image = cv::imread(filename[i]);
        cv::Mat contour_image = image.clone();
        using_minAreaRect(image,contour_image);
        using_approxPolyDP(image, contour_image);
        cv::imwrite(result_path[i], contour_image);
    }
}//g++ Plate.cpp -o Plate `pkg-config opencv4 --cflags --libs`

void using_minAreaRect(cv::Mat &image, cv::Mat &contour_image){
    cv::Mat channels[3];
    assert(image.channels() == 3);
    cv::split(image,channels);

    cv::Mat blue_sub_red_sub_green = channels[0] - channels[1] - channels[2];
    cv::Mat after_threshold,after_dilate;
    cv::Mat kernel=cv::getStructuringElement(cv::MORPH_CROSS,cv::Size(6,6));//定义形态化运算的结构元素
    cv::threshold(blue_sub_red_sub_green, after_threshold, 50, 255, cv::THRESH_BINARY);
    cv::dilate(after_threshold, after_dilate, kernel);//膨胀运算使轮廓连接起来
    //cv::imshow("blue", blue_sub_red_sub_green);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(after_dilate, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    //cv::drawContours(contourImage, contours, -1, cv::Scalar(0,0,255),2);
    
    for(const auto& contour : contours){
        if(cv::contourArea(contour) > 1000){
            cv::RotatedRect boundingRect = cv::minAreaRect(contour);//最小外接矩形
            cv::Point2f rectPoints[4];
            cv::Size2f rectSize = boundingRect.size;
            float width = rectSize.width;
            float height = rectSize.height;
            boundingRect.points(rectPoints);
            if(height/width > 2.5 || height/width < 0.44)
                for (int j = 0; j < 4; ++j) {
                    cv::line(contour_image, rectPoints[j], rectPoints[(j + 1) % 4], cv::Scalar(0, 0, 255), 2);
                }//绘制矩形
        }//根据面积筛选轮廓
    }
    cv::imshow("contour", contour_image);
}

void using_approxPolyDP(cv::Mat &image, cv::Mat &contour_image) {
cv::Mat hsv_image, blue_image;
    cv::Mat kernel_first_erode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2));
    cv::Mat kernel_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20,20));
    cv::Mat kernel_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10,10));
    cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);
    assert(image.channels() == 3);
    cv::inRange(hsv_image, cv::Scalar(100, 200, 125), cv::Scalar(124, 255, 255), blue_image);
    //cv::morphologyEx(blue_image, blue_image, cv::MORPH_OPEN, kernel_open);
    //cv::morphologyEx(blue_image, blue_image, cv::MORPH_CLOSE, kernel_close);
    cv::erode(blue_image, blue_image, kernel_first_erode);
    cv::dilate(blue_image, blue_image, kernel_close);
    //cv::erode(blue_image, blue_image, kernel_open);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> max_contour;
    double max_contour_size=0;
    cv::findContours(blue_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    for(const auto& contour : contours){
        if(cv::contourArea(contour) > max_contour_size) {
            max_contour_size = cv::contourArea(contour);
            max_contour = contour;
        }
    }//找出面积最大的轮廓（但是无法处理多个车牌的情况）
    double epsilon = 0.05 * cv::arcLength(max_contour, true);
    std::vector<cv::Point> approx;
    cv::approxPolyDP(max_contour, approx, epsilon, true);
    cv::drawContours(contour_image, std::vector<std::vector<cv::Point>>{approx}, 0, cv::Scalar(0, 255, 0), 2);
            
    cv::imshow("contour", contour_image);
    cv::waitKey(0); 
}
