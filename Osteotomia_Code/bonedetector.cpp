
#include "bonedetector.h"

BoneDetector::BoneDetector()
{
    // default values

    // canny threasholds
    thr1_ = 50;
    thr2_ = 150;

    // minimun and maximum width of the searched template
    wmin_ = 500;
    wmax_ = 2000;
    // increasing step in pixels of the width
    rangeS_ = 50;
    // side of the squares in which the image is divided
    rangeXY_ = 50;

    // min value allowed is -pi
    phimin_ = -PI;
    // max value allowed is +pi
    phimax_ = +PI;
    // number of slices (angles) in R-table
    intervals_ = 16;
}

void
BoneDetector::setTresholds(int t1, int t2)
{
    thr1_ = t1;
    thr2_ = t2;
}

void
BoneDetector::setLinearPars(int w1, int w2, int rS, int rXY)
{
    wmin_ = w1;
    wmax_ = w2;
    rangeS_ = rS;
    rangeXY_ = rXY;
}

void
BoneDetector::setAngularPars(float p1, float p2, int ints)
{
    if (p1<p2)
    {
        if (p1>-PI)
        {
            phimin_ = p1;
        }
        if (p2<+PI)
        {
            phimax_ = p2;
        }
    }
    intervals_ = ints;
}

// fill accumulator matrix
void
BoneDetector::accumulate(cv::Mat& input_img)
{
    cv::Mat detected_edges(input_img);
    // get Scharr matrices from image to obtain contour gradients
    cv::Mat dx;
    dx.create( cv::Size(input_img.cols, input_img.rows), CV_16SC1);
    cv::Sobel(detected_edges, dx, CV_16S, 1, 0, CV_SCHARR);
    cv::Mat dy;
    dy.create( cv::Size(input_img.cols, input_img.rows), CV_16SC1);
    cv::Sobel(detected_edges, dy, CV_16S, 0, 1, CV_SCHARR);
    // load all points from image all image contours on vector pts2
    int nl= detected_edges.rows;
    int nc= detected_edges.cols;
    float deltaphi = PI/intervals_;
    float inv_deltaphi = (float)intervals_/PI;
    float inv_rangeXY = (float)1/rangeXY_;
    float pi_half = PI*0.5f;
    std::vector<Rpoint2> pts2;
    for (int j=0; j<nl; ++j)
    {
        uchar* data= (uchar*)(detected_edges.data + detected_edges.step.p[0]*j);
        for (int i=0; i<nc; ++i)
        {
            if ( data[i]==255 ) // consider only white points (contour)
            {
                short vx = dx.at<short>(j,i);
                short vy = dy.at<short>(j,i);
                Rpoint2 rpt;
                rpt.x = i*inv_rangeXY;
                rpt.y = j*inv_rangeXY;
                float a = atan2((float)vy, (float)vx);              //	gradient angle in radians
                float phi = ((a > 0) ? a-pi_half : a+pi_half);      // contour angle with respect to x axis
                int angleindex = (int)((phi+PI*0.5f)*inv_deltaphi); // index associated with angle (0 index = -90 degrees)
                if (angleindex == intervals_) angleindex=intervals_-1;// -90°angle and +90° has same effect
                rpt.phiindex = angleindex;
                pts2.push_back( rpt );
            }
        }
    }
    // OpenCv 4-dimensional matrix definition and in general a useful way for defining multidimensional arrays and vectors in c++
    // create accumulator matrix
    int X = ceil((float)nc/rangeXY_);
    int Y = ceil((float)nl/rangeXY_);
    int S = ceil((float)(wmax_-wmin_)/rangeS_+1.0f);
    int R = ceil(phimax_/deltaphi)-floor(phimin_/deltaphi);
    if (phimax_==PI && phimin_==-PI) R--;
    int r0 = -floor(phimin_/deltaphi);
    int matSizep_S[] = {X, Y, S, R};
    accum_.create(4, matSizep_S, CV_16S);
    accum_ = cv::Scalar::all(0);
    // icrease accum_ cells with hits corresponding with slope in Rtable vector rotatated and scaled
    float inv_wtemplate_rangeXY = (float)1/(wtemplate_*rangeXY_);
    // rotate RTable from minimum to maximum angle
    for (int r=0; r<R; ++r)
    {  // rotation
        int reff = r-r0;
        std::vector<std::vector<cv::Vec2f>> Rtablerotated(intervals_);
        // cos and sin are computed in the outer loop to reach computational efficiency
        float cs = cos(reff*deltaphi);
        float sn = sin(reff*deltaphi);
        for (std::vector<std::vector<cv::Vec2i>>::size_type ii = 0; ii < Rtable.size(); ++ii)
        {
            for (std::vector<cv::Vec2i>::size_type jj= 0; jj < Rtable[ii].size(); ++jj)
            {
                int iimod = (ii+reff) % intervals_;
                Rtablerotated[iimod].push_back(cv::Vec2f(cs*Rtable[ii][jj][0] - sn*Rtable[ii][jj][1], sn*Rtable[ii][jj][0] + cs*Rtable[ii][jj][1]));
            }
        }
        // scale the rotated RTable from minimum to maximum scale
        for (int s=0; s<S; ++s)
        {  // scale
            std::vector<std::vector<cv::Vec2f>> Rtablescaled(intervals_);
            int w = wmin_ + s*rangeS_;
            float wratio = (float)w*inv_wtemplate_rangeXY;
            //cout << "Ratio: " << wratio << endl;
            for (std::vector<std::vector<cv::Vec2f>>::size_type ii = 0; ii < Rtablerotated.size(); ++ii)
            {
                for (std::vector<cv::Vec2f>::size_type jj= 0; jj < Rtablerotated[ii].size(); ++jj)
                {
                    Rtablescaled[ii].push_back(cv::Vec2f(wratio*Rtablerotated[ii][jj][0], wratio*Rtablerotated[ii][jj][1]));
                }
            }
            // iterate through each point of edges and hit corresponding cells from rotated and scaled Rtable
            for (std::vector<Rpoint2>::size_type t = 0; t < pts2.size(); ++t)
            { // XY plane
                int angleindex = pts2[t].phiindex;
                for (std::vector<cv::Vec2f>::size_type index = 0; index < Rtablescaled[angleindex].size(); ++index)
                {
                    float deltax = Rtablescaled[angleindex][index][0];
                    float deltay = Rtablescaled[angleindex][index][1];
                    int xcell = (int)(pts2[t].x + deltax);
                    int ycell = (int)(pts2[t].y + deltay);
                    if ( (xcell<X)&&(ycell<Y)&&(xcell>-1)&&(ycell>-1) )
                    {
                        //(*( (short*)(accum_.data + xcell*accum_.step.p[0] + ycell*accum_.step.p[1] + s*accum_.step.p[2]+ r*accum_.step.p[3])))++;
                        (*ptrat4D(accum_, xcell, ycell, s, r))++;
                    }
                }
            }
        }
    }
}

double
BoneDetector::getAng(int r)
{
    double deltaphi = PI/intervals_;
    double r0 = std::floor(phimin_/deltaphi);
    double reff = static_cast<double>(r)+r0;
    return (reff*deltaphi);
}

double
BoneDetector::getRatio(int s)
{
    int w = wmin_ + s*rangeS_;
    return (static_cast<double>(w)/wtemplate_);
}

cv::Vec2i
BoneDetector::getPointA (int x, int y)
{
    return cv::Vec2i(x*rangeXY_+(rangeXY_+1)/2, y*rangeXY_+(rangeXY_+1)/2);
}

cv::Vec2i
BoneDetector::getPointB (cv::Vec2i pointA, double ang, double ratio)
{
    return Utils::rotatePoint(pointA + ((refPointB_ - refPointA_) * ratio), pointA, ang);
}

// show the best candidate detected on image
double
BoneDetector::bestCandidate(Bone bone, cv::Vec2i& pointA, cv::Vec2i& pointB, float& ang, float& ratio)
{
    int xBegin = 0, xEnd = 0, yBegin = 0, yEnd = 0;
    switch(bone)
    {
        case Bone::RIGHT_FEMUR:
        {
            xBegin = 2;
            xEnd = accum_.size[0]/2;
            yBegin = 2;
            yEnd = accum_.size[1]/2;
        }
        break;
        case Bone::RIGHT_TIBIA:
        {
            xBegin = 2;
            xEnd = accum_.size[0]/2;
            yBegin = accum_.size[1]/2;
            yEnd = accum_.size[1] - 2;
        }
        break;
        case Bone::LEFT_FEMUR:
        {
            xBegin = accum_.size[0]/2;
            xEnd = accum_.size[0] - 2;
            yBegin = 2;
            yEnd = accum_.size[1]/2;
        }
        break;
        case Bone::LEFT_TIBIA:
        {
            xBegin = accum_.size[0]/2;
            xEnd = accum_.size[0] - 2;
            yBegin = accum_.size[1]/2;
            yEnd = accum_.size[1] - 2;
        }
        break;
    }

    int cx = 0;
    int cy = 0;
    int cs = 0;
    int cr = 0;
    int cv = 0;
    for (int x = xBegin; x < xEnd; x++)
    {
        for (int y = yBegin; y < yEnd; y++)
        {
            for (int s = 0; s < accum_.size[2]; s++)
            {
                for (int r = 0; r < accum_.size[3]; r++)
                {
                    int v = (*ptrat4D(accum_, x, y, s, r));
                    if (v > cv)
                    {
                        double an = getAng(r);
                        double ra = getRatio(s);
                        cv::Vec2i pA = getPointA(x,y);
                        cv::Vec2i pB = getPointB(pA, an, ra);
                        if(pB[0] > accum_.size[0] || pB[1] > accum_.size[1])
                        {
                            continue;
                        }
                        cx = x;
                        cy = y;
                        cs = s;
                        cr = r;
                        cv = v;
                    }
                }
            }
        }
    }
    ang = getAng(cr);
    ratio = getRatio(cs);
    pointA = getPointA(cx,cy);
    pointB = getPointB(pointA, ang, ratio);

    return static_cast<double>(cv) / static_cast<double>(pts_.size());
}

void
BoneDetector::createRtable(const char* templatePointsPath, int flags)
{
    cv::Mat mat(cv::imread(templatePointsPath, flags));
    readPoints(mat);
    readRtable();
}

void
BoneDetector::createRtable(cv::Mat& templateImage)
{
    readPoints(templateImage);
    readRtable();
}

// load vector pts_ with all points from the contour
void
BoneDetector::readPoints(cv::Mat& templateImage)
{
    cv::Vec3b refAPoint(cv::Vec3b(127, 127, 127));
    cv::Vec3b refBPoint(cv::Vec3b(63, 63, 63));
    refPointA_ = getReferencePoint(templateImage, refAPoint);
    refPointB_ = getReferencePoint(templateImage, refBPoint);
    wtemplate_ = loadPointsPoint(templateImage, pts_);
}

cv::Vec2i
BoneDetector::getReferencePoint(cv::Mat& templateImage, cv::Vec3b& color)
{
    // find reference point inside contour image and save it in variable refPoint
    int nl = templateImage.rows;
    int nc = templateImage.cols;
    for (int j = 0; j<nl; ++j)
    {
        cv::Vec3b* data = (cv::Vec3b*) (templateImage.data + templateImage.step.p[0] * j);
        for (int i = 0; i<nc; ++i)
        {
            if (data[i] == color)
            {
                return cv::Vec2i(i, j);
            }
        }
    }
    return cv::Vec2i(-1, -1);
}

int
BoneDetector::loadPointsPoint(cv::Mat& templateImage, std::vector<Rpoint>& points)
{
    // load points on vector
    points.clear();
    int mindx = INT_MAX;
    int maxdx = INT_MIN;
    int nl = templateImage.rows;
    int nc = templateImage.cols;
    for (int j=0; j<nl; ++j)
    {
        cv::Vec3b* data= (cv::Vec3b*)(templateImage.data + templateImage.step.p[0]*j);
        for (int i=0; i<nc; ++i)
        {
            if ( data[i] == cv::Vec3b(255,255,255) )
            {
                Rpoint rpt;
                rpt.dx = refPointA_(0)-i;
                rpt.dy = refPointA_(1)-j;
                rpt.phi = PI/2;
                // update further right and left dx
                if (rpt.dx < mindx) mindx=rpt.dx;
                if (rpt.dx > maxdx) maxdx=rpt.dx;
                points.push_back(rpt);
            }
        }
    }
    // maximum width of the contour
    return maxdx-mindx+1;
}



// create Rtable from contour points
void
BoneDetector::readRtable()
{
    Rtable.clear();
    Rtable.resize(intervals_);
    // put points in the right interval, according to discretized angle and range size
    float range = PI/intervals_;
    for (std::vector<Rpoint>::size_type t = 0; t < pts_.size(); ++t)
    {
        int angleindex = (int)((pts_[t].phi+PI/2)/range);
        if (angleindex == intervals_) angleindex=intervals_-1;
        Rtable[angleindex].push_back( cv::Vec2i(pts_[t].dx, pts_[t].dy) );
    }
}

