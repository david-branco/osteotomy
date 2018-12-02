#ifndef UTILS_H
#define UTILS_H

#include <QFile>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include <functional>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

class Utils
{
public:
    Utils();
    static bool loadImageFromResource(cv::Mat& mat, const char* resourcePath, int flag = cv::IMREAD_UNCHANGED);
    static std::vector<std::string> split(const std::string &s, char delim);
    static cv::Rect getGoodRect(cv::Mat* image, int width, int height);
    static void print(cv::Rect& roi);
    static void print(cv::Mat& mat);
    static bool equals(cv::Rect* a, cv::Rect* b);
    static cv::Point rotate2d(const cv::Point& inPoint, const double& angRad);
    static cv::Point rotatePoint(const cv::Point& inPoint, const cv::Point& center, const double& angRad);
    static cv::Vec2i rotate2d(const cv::Vec2i& inPoint, const double& angRad);
    static cv::Vec2i rotatePoint(const cv::Vec2i& inPoint, const cv::Vec2i& center, const double& angRad);
};

#endif // UTILS_H
