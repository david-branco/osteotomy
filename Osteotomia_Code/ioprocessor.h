#ifndef IOPROCESSOR_H
#define IOPROCESSOR_H

#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <functional>
#include <events.h>

class IOProcessor
{
private:
    std::function<void(Events,void*)> eventCallback_;
    QPoint clickOffset_;
    QPoint moveOffset_;
public:
    IOProcessor(std::function<void(Events,void*)> eventCallback);
    void wheelEvent (QWheelEvent * event);
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // IOPROCESSOR_H
