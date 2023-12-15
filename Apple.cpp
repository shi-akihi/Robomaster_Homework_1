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

int main()
{
	cv::Mat apple = cv::imread("apple.png");
	assert(apple.channels() == 3);
    cv::Mat channels[3];
    cv::split(apple,channels);//分成三个通道
    cv::Mat red_sub_green_sub_blue = channels[2] - channels[1] - channels[0];
    cv::Mat normal_mat;//红色通道减去绿色及蓝色通道后归一化的矩阵
    cv::Mat result;//处理完的矩阵
    cv::Mat kernel=cv::getStructuringElement(cv::MORPH_CROSS,cv::Size(6,6));//定义形态化运算的结构元素
    cv::normalize(red_sub_green_sub_blue, normal_mat, 0., 255., cv::NORM_MINMAX);//红色通道减去绿色及蓝色通道
    cv::threshold(red_sub_green_sub_blue, result, 15, 255, cv::THRESH_BINARY);//固定阈值二值化
    cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel);//开运算
    cv::dilate(result, result, kernel);//膨胀运算使轮廓连接起来
    //cv::adaptiveThreshold(normal_mat, result, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 21, 0);
    cv::Mat contours_image = apple.clone();//定义绘制轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(result, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);//寻找轮廓
    for(const auto& contour : contours){
        if(cv::contourArea(contour) > 1000){
            cv::drawContours(contours_image, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(255, 255, 255), 2);//注意contours是一个二维向量
            cv::RotatedRect boundingRect = cv::minAreaRect(contour);//最小外接矩形
            cv::Point2f rectPoints[4];
            boundingRect.points(rectPoints);
            for (int j = 0; j < 4; ++j) {
                cv::line(contours_image, rectPoints[j], rectPoints[(j + 1) % 4], cv::Scalar(255, 255, 255), 2);
            }//绘制矩形
        }//根据面积筛选轮廓
    }//遍历所有轮廓
    cv::imshow("result",contours_image);
    cv::waitKey(0);
    return 0;
}//`pkg-config opencv4 --cflags --libs`
//g++ Apple.cpp -o Apple `pkg-config opencv4 --cflags --libs`