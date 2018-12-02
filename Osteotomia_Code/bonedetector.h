#ifndef BONEDETECTOR_H
#define BONEDETECTOR_H

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include "config.h"
#include "utils.h"

class BoneDetector
{
private:
    struct Rpoint
    {
        int dx;
        int dy;
        float phi;
    };

    struct Rpoint2
    {
        float x;
        float y;
        int phiindex;
    };

    cv::Mat accum_;
    //cv::Mat showimage_;
    std::vector<Rpoint> pts_;
    cv::Vec2i refPointA_;
    cv::Vec2i refPointB_;
    std::vector<std::vector<cv::Vec2i>> Rtable;
    int intervals_;
    int thr1_;
    int thr2_;
    int wtemplate_;
    int wmin_;
    int wmax_;
    double phimin_;
    double phimax_;
    int rangeXY_;
    int rangeS_;

public:
    enum Bone
    {
        RIGHT_FEMUR = 0,
        RIGHT_TIBIA,
        LEFT_FEMUR,
        LEFT_TIBIA
    };

    BoneDetector();

    void setTresholds(int t1, int t2);
    void setLinearPars(int w1, int w2, int rS, int rXY);
    void setAngularPars(float p1, float p2, int ints);
    void createRtable(const char* templatePointsPath, int flags);
    void createRtable(cv::Mat& templateImage);
    void accumulate(cv::Mat& input_img);
    double bestCandidate(Bone bone, cv::Vec2i& pointA, cv::Vec2i& pointB, float& ang, float& ratio);

private:

    void readRtable();
    void readPoints(cv::Mat& templateImage);
    cv::Vec2i getReferencePoint(cv::Mat& templateImage, cv::Vec3b& color);
    int loadPointsPoint(cv::Mat& templateImage, std::vector<Rpoint>& points);

    double getAng(int r);
    double getRatio(int s);

    cv::Vec2i getPointA (int x, int y);
    cv::Vec2i getPointB (cv::Vec2i pointA, double ang, double ratio);

    inline int roundToInt(float num)
    {
        return (num > 0.0) ? (int)(num + 0.5f) : (int)(num - 0.5f);
    }

    inline short at4D(cv::Mat &mt, int i0, int i1, int i2, int i3)
    {
        return *( (short*)(mt.data + i0*mt.step.p[0] + i1*mt.step.p[1] + i2*mt.step.p[2] + i3*mt.step.p[3]));
    }

    inline short* ptrat4D(cv::Mat &mt, int i0, int i1, int i2, int i3)
    {
        return (short*)(mt.data + i0*mt.step.p[0] + i1*mt.step.p[1] + i2*mt.step.p[2] + i3*mt.step.p[3]);
    }

};

#endif // BONEDETECTOR_H
