#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <opencv2/opencv.hpp>
#include "config.h"
#include "utils.h"
#include "cvimagewidget.h"
#include "dicomloader.h"
#include "layoutwindow.h"
#include "ioprocessor.h"
#include "events.h"
#include "edgedetector.h"
#include "bonedetector.h"
#include "windowdialog.h"

typedef std::pair<cv::Vec2i, cv::Vec2i> Vec2iPair;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    struct Point
    {
    public:
        float x;
        float y;
        Point() : x(0.0f),y(0.0f){}
        Point(float vx, float vy) : x(vx),y(vy){}
        Point(int vx, int vy)
        {
            x = static_cast<float>(vx);
            y = static_cast<float>(vy);
        }
        Point& operator=(const Point& point)
        {
            x = point.x;
            y = point.y;
            return *this;
        }
        Point& operator=(cv::Point& point)
        {
            x = point.x;
            y = point.y;
            return *this;
        }
        int intX()
        {
            return static_cast<int>(std::round(x));
        }
        int intY()
        {
            return static_cast<int>(std::round(y));
        }
        void devideBy(float value)
        {
            x /= value;
            y /= value;
        }
        void multiplyBy(float value)
        {
            x *= value;
            y *= value;
        }

        cv::Point toCv()
        {
            return cv::Point(intX(),intY());
        }
    };

    enum Leg{
        LEFT,
        RIGHT
    };

protected:
     void wheelEvent ( QWheelEvent * event );
     void keyPressEvent( QKeyEvent *event );
     void mouseMoveEvent(QMouseEvent * event);
     void mousePressEvent(QMouseEvent * event);
     void mouseReleaseEvent(QMouseEvent * event);
     void resizeEvent(QResizeEvent* event);
private:
    Ui::MainWindow *ui;

    std::shared_ptr<DicomImage> dicomImage_;
    std::shared_ptr<IOProcessor> ioProcessor_;
    std::shared_ptr<CVImageWidget> imageWidget_;
    std::shared_ptr<cv::Mat> loadedImage_;
    std::shared_ptr<LayoutWindow> layoutWindow_;
    std::shared_ptr<QLabel> statusLabel_;
    std::shared_ptr<cv::Rect> positionAndSize_;
    std::shared_ptr<cv::Rect> displayRoi_;
    std::shared_ptr<std::vector<std::shared_ptr<Point>>> points_;
    std::shared_ptr<QSettings> settings_;
    int minWidth_;
    int visibleWidth_;
    float px_;
    float py_;
    int lastIdx_;
    float zoomFactor_;
    Leg leg_;



public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void eventHandler(Events event, void* parameters);

private:
    void initialize(IplImage* image);
    void setMatToCorrectAspectRatio (cv::Mat* image, cv::Rect* positionAndSize);
    cv::Mat getImage( cv::Mat& img, float& x, float& y, int width, int target_width, int target_height, cv::Rect* roi);
    void updateScreenImage();
    void loadImage(const wchar_t* filename);
    void saveImage(cv::Mat& image, const char* filename);
    int mouseOverPoint (cv::Point mousePoint);
    cv::Point screenPointToWorldPoint (cv::Point point);
    cv::Point worldPointToScreenPoint (cv::Point point);
    void centerImage();
    void clearPoints();
    void updateStatus();
    void drawGraphics(cv::Mat* image, bool adjustToScreen = true);
    void convertTo8BitColor(cv::Mat& src, cv::Mat& dst);
    void automatic();
    void addPoint( int x, int y);
    Vec2iPair getAutoPoint( cv::Mat& bone, cv::Mat& image, BoneDetector::Bone boneName, double& factor);
    void loadSettings(const char* filePath);
};

#endif // MAINWINDOW_H
