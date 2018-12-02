#ifndef LAYOUTWINDOW_H
#define LAYOUTWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QFileDialog>

#include <functional>

#include "cvimagewidget.h"
#include "config.h"
#include "events.h"

class LayoutWindow
{
private:
    QMenu *fileMenu_;
    QAction *openAct_;
    QAction *saveAct_;
    QAction *exitAct_;

    QMenu *editMenu_;
    QAction *centerAct_;
    QAction *clearAct_;
    QMenu *legMenu_;
    QAction *legLAct_;
    QAction *legRAct_;
    QAction *settingsAct_;
    QAction *calcAct_;

    QMenu *helpMenu_;
    QAction *helpAct_;
    QAction *aboutAct_;

public:
    LayoutWindow(QMainWindow* window, CVImageWidget* imageWidget, std::function<void(Events,void*)> eventCallback);
    void setEnabled(bool value);

private:
    void close();
    void createActions(QMainWindow* window, std::function<void(Events,void*)> eventCallback);
    void createMenus(QMainWindow* window);

private slots:
    void open();

};

#endif // LAYOUTWINDOW_H
