#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>

class CVImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CVImageWidget(QWidget *parent = 0) : QWidget(parent)
    {
    }

    virtual ~CVImageWidget()
    {
    }

    QSize sizeHint() const
    {
        return qimage_.size();
    }
    QSize minimumSizeHint() const
    {
        return qimage_.size();
    }

public slots:

    void showImage(const cv::Mat& image)
    {
        // Convert the image to the RGB888 format
        switch (image.type())
        {
        case CV_8UC1:
            cv::cvtColor(image, tmp_, CV_GRAY2RGB);
            break;
        case CV_8UC3:
            cv::cvtColor(image, tmp_, CV_BGR2RGB);
            break;
        default: assert(false && "Image type is not supported!");
        }
        // QImage needs the data to be stored continuously in memory
        assert(tmp_.isContinuous());
        // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
        // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
        // has three bytes.
        qimage_ = QImage(tmp_.data, tmp_.cols, tmp_.rows, tmp_.cols*3, QImage::Format_RGB888);
        this->setFixedSize(image.cols, image.rows);
        repaint();
    }

protected:
    void paintEvent(QPaintEvent* /*event*/)
    {
        // Display the image
        QPainter painter(this);
        painter.drawImage(QPoint(0,0), qimage_);
        painter.end();
    }

    QImage qimage_;
    cv::Mat tmp_;
};

#endif // CVIMAGEWIDGET_H
