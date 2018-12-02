#ifndef EDGEDETECTOR_H
#define EDGEDETECTOR_H

#include <opencv2/opencv.hpp>

class EdgeDetector
{
private:
    int horizontalSize_;
    int verticalSize_;
    int blurSize_;
    int cannyThreshold1_;
    int cannyThreshold2_;

    int claheClipLimit_;
    int claheTilesGridSize_;

public:
    EdgeDetector(int horizontalSize = 10,
                 int verticalSize = 16,
                 int blurSize = 3,
                 int cannyThreshold1 = 14,
                 int cannyThreshold2 = 35,
                 int claheClipLimit = 4,
                 int claheTilesGridSize = 1);
    void process(cv::Mat& src, cv::Mat& dst);

    int horizontalSize();
    int verticalSize();
    int blurSize();
    int cannyThreshold1();
    int cannyThreshold2();
    int claheClipLimit();
    int claheTilesGridSize();

    void horizontalSize(int value);
    void verticalSize(int value);
    void blurSize(int value);
    void cannyThreshold1(int value);
    void cannyThreshold2(int value);
    void claheClipLimit(int value);
    void claheTilesGridSize(int value);

private:
    void removeLines (cv::Mat& src, cv::Mat& dst, cv::Size size);
    void removeHorizontalLines (cv::Mat& src, cv::Mat& dst, int horizontalSize);
    void removeVerticalLines (cv::Mat& src, cv::Mat& dst, int verticalSize);
};

#endif // EDGEDETECTOR_H
