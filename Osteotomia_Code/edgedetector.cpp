#include "edgedetector.h"

EdgeDetector::EdgeDetector(int horizontalSize,
                           int verticalSize,
                           int blurSize,
                           int cannyThreshold1,
                           int cannyThreshold2,
                           int claheClipLimit,
                           int claheTilesGridSize)
{
    this->horizontalSize(horizontalSize);
    this->verticalSize(verticalSize);
    this->blurSize(blurSize);
    this->cannyThreshold1(cannyThreshold1);
    this->cannyThreshold2(cannyThreshold2);
    this->claheClipLimit(claheClipLimit);
    this->claheTilesGridSize(claheTilesGridSize);
}

void
EdgeDetector::removeLines (cv::Mat& src, cv::Mat& dst, cv::Size size)
{
    dst = src.clone();
    if(size.width * size.height <= 1)
    {
        return;
    }
    cv::Mat structure = getStructuringElement(cv::MORPH_RECT, size);
    erode(dst, dst, structure, cv::Point(-1, -1));
    dilate(dst, dst, structure, cv::Point(-1, -1));
}

void
EdgeDetector::removeHorizontalLines (cv::Mat& src, cv::Mat& dst, int horizontalSize)
{
    if(horizontalSize > 0)
    {
        removeLines(src, dst, cv::Size(1, horizontalSize));
    }
    else
    {
        src.copyTo(dst);
    }
}

void
EdgeDetector::removeVerticalLines (cv::Mat& src, cv::Mat& dst, int verticalSize)
{
    if(verticalSize > 0)
    {
        removeLines(src, dst, cv::Size(verticalSize, 1));
    }
    else
    {
        src.copyTo(dst);
    }
}

void
EdgeDetector::process(cv::Mat& src, cv::Mat& dst)
{
#ifndef QT_NO_DEBUG
    cv::imshow("1 Original",src);
#endif

    if(claheTilesGridSize_ > 0)
    {
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
        clahe -> setClipLimit(claheClipLimit_);
        clahe -> setTilesGridSize(cv::Size(claheTilesGridSize_,claheTilesGridSize_));
        clahe -> apply(src,dst);
    }
    else
    {
        src.copyTo(dst);
    }


#ifndef QT_NO_DEBUG
    cv::imshow("2 CLAHE",dst);
#endif

    removeHorizontalLines (dst, dst, horizontalSize_);
    removeVerticalLines (dst, dst, verticalSize_);

#ifndef QT_NO_DEBUG
    cv::imshow("3 H/V Lines removed",dst);
#endif

    if(blurSize_ > 0)
    {
        cv::GaussianBlur(dst, dst, cv::Size(blurSize_, blurSize_), 0);
    }
    else
    {
        src.copyTo(dst);
    }

#ifndef QT_NO_DEBUG
    cv::imshow("4 Gaussian Blur",dst);
#endif

    cv::Canny(dst, dst, cannyThreshold1_, cannyThreshold2_);

#ifndef QT_NO_DEBUG
    cv::imshow("5 Canny",dst);
#endif
}

int
EdgeDetector::horizontalSize()
{
    return horizontalSize_;
}

int
EdgeDetector::verticalSize()
{
    return verticalSize_;
}

int
EdgeDetector::blurSize()
{
    return blurSize_;
}

int
EdgeDetector::cannyThreshold1()
{
    return cannyThreshold1_;
}

int
EdgeDetector::cannyThreshold2()
{
    return cannyThreshold2_;
}

int
EdgeDetector::claheClipLimit()
{
    return claheClipLimit_;
}

int
EdgeDetector::claheTilesGridSize()
{
    return claheTilesGridSize_;
}

void
EdgeDetector::horizontalSize(int value)
{
    horizontalSize_ = value;
}

void
EdgeDetector::verticalSize(int value)
{
    verticalSize_ = value;
}

void
EdgeDetector::blurSize(int value)
{
    assert(value >= 0);
    if(value%2 == 0)
    {
        ++value;
    }
    blurSize_ = value;
}

void
EdgeDetector::cannyThreshold1(int value)
{
    cannyThreshold1_ = value;
}

void
EdgeDetector::cannyThreshold2(int value)
{
    cannyThreshold2_ = value;
}

void
EdgeDetector::claheClipLimit(int value)
{
    claheClipLimit_ = value;
}

void
EdgeDetector::claheTilesGridSize(int value)
{
    claheTilesGridSize_ = value;
}

