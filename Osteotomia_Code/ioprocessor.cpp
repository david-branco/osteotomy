#include "ioprocessor.h"

IOProcessor::IOProcessor(std::function<void(Events,void*)> eventCallback) :
    eventCallback_(eventCallback),
    clickOffset_(-1,-1),
    moveOffset_(-1,-1)
{

}

void IOProcessor::wheelEvent ( QWheelEvent * event )
{
    int delta = event->delta();
    unsigned int numOfZoom = static_cast<unsigned int>(std::abs(delta/120));
    eventCallback_(delta > 0 ? Events::ZOOM_IN : Events::ZOOM_OUT, static_cast<void*>(&numOfZoom));
}

void IOProcessor::keyPressEvent( QKeyEvent *event )
{
    switch ( event->key() )
    {
        case Qt::Key_Up:
        case Qt::Key_W:
            eventCallback_(Events::MOVE_UP, NULL);
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            eventCallback_(Events::MOVE_DOWN, NULL);
            break;
        case Qt::Key_Left:
        case Qt::Key_A:
            eventCallback_(Events::MOVE_LEFT, NULL);
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            eventCallback_(Events::MOVE_RIGHT, NULL);
            break;
        case Qt::Key_Space:
            eventCallback_(Events::CENTER, NULL);
            break;
        case Qt::Key_Plus:
            eventCallback_(Events::ZOOM_IN, NULL);
            break;
        case Qt::Key_Minus:
            eventCallback_(Events::ZOOM_OUT, NULL);
            break;
        case Qt::Key_Escape:
            exit(0);
            break;
        case Qt::Key_C:
            eventCallback_(Events::CLEAR_POINTS, NULL);
            break;
    }
}

void IOProcessor::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QPoint offset = event->pos();
        QPoint move = offset - moveOffset_;
        std::pair<int,int> delta(move.x(),move.y());
        std::pair<int,int> position(offset.x(),offset.y());
        std::pair<std::pair<int,int>,std::pair<int,int>> data(delta, position);
        eventCallback_(Events::MOVE, static_cast<void*>(&data));
        moveOffset_ = offset;
    }
}

void IOProcessor::mousePressEvent(QMouseEvent *event)
{
    clickOffset_ = event->pos();
    moveOffset_ = clickOffset_;
}

void IOProcessor::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint position = event->pos();
    std::pair<int,int> mousePosition(position.x(),position.y());
    QPoint point = position - clickOffset_;
    switch(event->button())
    {
        case Qt::LeftButton:
        {
            eventCallback_(MOUSE_LEFT_RELEASE, (point.manhattanLength() < 3 ? static_cast<void*>(&mousePosition) : NULL));
        }
        break;
        case Qt::RightButton:
        {
            eventCallback_(MOUSE_RIGHT_RELEASE, (point.manhattanLength() < 3 ? static_cast<void*>(&mousePosition) : NULL));
        }
        break;
        default:
            ;
    }
}


