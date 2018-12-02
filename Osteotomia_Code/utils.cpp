#include "utils.h"

Utils::Utils()
{

}


bool Utils::loadImageFromResource(cv::Mat& mat, const char* resourcePath, int flag)
{
    mat.release();
    QFile file(resourcePath);
    if(file.open(QIODevice::ReadOnly))
    {
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*)buf.data(), sz);
        mat = cv::imdecode(buf, flag);
        return true;
    }
    return false;
}

std::vector<std::string> Utils::split(const std::string &s, char delim)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

cv::Rect Utils::getGoodRect(cv::Mat* image, int width, int height)
{
    std::function<int(int,int)> gcd = [&](int a, int b)
    {
        return b == 0 ? a : gcd(b, a % b);
    };

    int w = image->cols;
    int h = image->rows;

    int d = gcd (width, height);
    width /= d;
    height /= d;

    float wf = static_cast<float>(w) / static_cast<float>(width);
    float hf = static_cast<float>(h) / static_cast<float>(height);

    int max = wf > hf ? static_cast<int>(std::ceil(wf)) : static_cast<int>(std::ceil(hf));

    cv::Rect r;
    r.width = width * max;
    r.height = height * max;
    r.x = (r.width - w) / 2;
    r.y = (r.height - h) / 2;
    return r;
}

cv::Point Utils::rotate2d(const cv::Point& inPoint, const double& angRad)
{
    cv::Point outPoint;
    outPoint.x = std::cos(angRad)*inPoint.x - std::sin(angRad)*inPoint.y;
    outPoint.y = std::sin(angRad)*inPoint.x + std::cos(angRad)*inPoint.y;
    return outPoint;
}

cv::Point Utils::rotatePoint(const cv::Point& inPoint, const cv::Point& center, const double& angRad)
{
    return Utils::rotate2d(inPoint - center, angRad) + center;
}

cv::Vec2i Utils::rotate2d(const cv::Vec2i& inPoint, const double& angRad)
{
    cv::Vec2i outPoint;
    outPoint[0] = std::cos(angRad)*inPoint[0] - std::sin(angRad)*inPoint[1];
    outPoint[1] = std::sin(angRad)*inPoint[0] + std::cos(angRad)*inPoint[1];
    return outPoint;
}

cv::Vec2i Utils::rotatePoint(const cv::Vec2i& inPoint, const cv::Vec2i& center, const double& angRad)
{
    return Utils::rotate2d(inPoint - center, angRad) + center;
}

bool Utils::equals(cv::Rect* a, cv::Rect* b)
{
    return (a->x == b->x && a->y == b->y && a->width == b->width && a->height == b->height);
}

void Utils::print(cv::Mat& mat)
{
    std::cout << "W=" << mat.cols << " H=" << mat.rows << std::endl;
}

void Utils::print(cv::Rect& roi)
{
    std::cout << "x=" << roi.x << "y=" << roi.y << "w=" << roi.width << "h=" << roi.height << std::endl;
}
