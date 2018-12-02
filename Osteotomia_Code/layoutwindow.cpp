#include "layoutwindow.h"

LayoutWindow::LayoutWindow(QMainWindow* window, CVImageWidget* imageWidget, std::function<void(Events,void*)> eventCallback)
{
    window->setMinimumSize(MAIN_WINDOW_W+2*MAIN_WINDOW_MARGIN, MAIN_WINDOW_H+2*MAIN_WINDOW_MARGIN+60);

    QWidget *widget = new QWidget;
    window->setCentralWidget(widget);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(MAIN_WINDOW_MARGIN);
    layout->addWidget(imageWidget);
    layout->setAlignment(imageWidget, Qt::AlignHCenter);
    widget->setLayout(layout);

    createActions(window, eventCallback);
    createMenus(window);
}

void LayoutWindow::createActions(QMainWindow* window, std::function<void(Events,void*)> eventCallback)
{
    openAct_ = new QAction(window->tr("Open..."), window);
    openAct_->setShortcuts(QKeySequence::Open);
    openAct_->setStatusTip(window->tr("Open an existing file"));

    saveAct_ = new QAction(window->tr("Save as ..."), window);
    saveAct_->setShortcuts(QKeySequence::SaveAs);
    saveAct_->setStatusTip(window->tr("Save the current image"));

    exitAct_ = new QAction(window->tr("Exit"), window);
    exitAct_->setShortcuts(QKeySequence::Quit);
    exitAct_->setStatusTip(window->tr("Exit the application"));

    centerAct_ = new QAction(window->tr("Center image"), window);
    centerAct_->setStatusTip(window->tr("Center the image"));

    clearAct_ = new QAction(window->tr("Clear points"), window);
    clearAct_->setStatusTip(window->tr("Clear the points"));

    legLAct_ = new QAction(window->tr("Left"), window);
    legLAct_->setCheckable(true);
    legLAct_->setChecked(true);
    legLAct_->setStatusTip(window->tr("Select the left leg"));

    legRAct_ = new QAction(window->tr("Right"), window);
    legRAct_->setCheckable(true);
    legRAct_->setStatusTip(window->tr("Select the right leg"));

    settingsAct_ = new QAction(window->tr("Settings"), window);
    settingsAct_->setStatusTip(window->tr("Show settings panel"));

    calcAct_ = new QAction(window->tr("Calculate"), window);
    calcAct_->setStatusTip(window->tr("Calculate automatically the points"));

    helpAct_ = new QAction(window->tr("Help"), window);
    helpAct_->setStatusTip(window->tr("Instructions"));

    aboutAct_ = new QAction(window->tr("About"), window);
    aboutAct_->setStatusTip(window->tr("Informations about the application"));

    window->connect(openAct_, &QAction::triggered, window, [window,eventCallback]()
    {
        QString fileName = QFileDialog::getOpenFileName(window, window->tr("Open Dicom"), window->tr("./"), window->tr("Dicom Files (*.dcm *.dicom)"));
        if(fileName.size() > 1)
        {
            std::wstring filePath(fileName.toStdWString());
            eventCallback(Events::OPEN, static_cast<void*>(const_cast<wchar_t*>(filePath.c_str())));
        }
    });
    window->connect(saveAct_, &QAction::triggered, window, [window,eventCallback]()
    {
        QString fileName = QFileDialog::getSaveFileName(window, window->tr("Save Image"), window->tr("./"), window->tr("PNG Files (*.png)"));
        if(fileName.size() > 1)
        {
            if(fileName.toLower().indexOf(window->tr(".png")) == -1)
            {
                fileName.append(window->tr(".png"));
            }
            eventCallback(Events::SAVE_AS, static_cast<void*>(const_cast<char*>(fileName.toStdString().c_str())));
        }
    });
    window->connect(exitAct_, &QAction::triggered, window, []()
    {
        exit(0);
    });

    window->connect(centerAct_, &QAction::triggered, window, [eventCallback]()
    {
        eventCallback(Events::CENTER, NULL);
    });
    window->connect(clearAct_, &QAction::triggered, window, [eventCallback]()
    {
        eventCallback(Events::CLEAR_POINTS, NULL);
    });
    window->connect(legLAct_, &QAction::triggered, window, [this,eventCallback]()
    {
        legLAct_->setChecked(true);
        legRAct_->setChecked(false);
        eventCallback(Events::LEG_LEFT, NULL);
    });
    window->connect(legRAct_, &QAction::triggered, window, [this,eventCallback]()
    {
        legRAct_->setChecked(true);
        legLAct_->setChecked(false);
        eventCallback(Events::LEG_RIGHT, NULL);
    });

    window->connect(settingsAct_, &QAction::triggered, window, [eventCallback]()
    {
        eventCallback(Events::SETTINGS, NULL);
    });
    window->connect(calcAct_, &QAction::triggered, window, [eventCallback]()
    {
        eventCallback(Events::AUTOMATIC, NULL);
    });

    window->connect(helpAct_, &QAction::triggered, window, [eventCallback]()
    {
        eventCallback(Events::HELP, NULL);
    });
    window->connect(aboutAct_, &QAction::triggered, window, [eventCallback]()
    {
        eventCallback(Events::ABOUT, NULL);
    });
}

void LayoutWindow::createMenus(QMainWindow* window)
{
    fileMenu_ = window->menuBar()->addMenu(window->tr("File"));
    editMenu_ = window->menuBar()->addMenu(window->tr("Edit"));
    helpMenu_ = window->menuBar()->addMenu(window->tr("About"));

    fileMenu_->addAction(openAct_);
    fileMenu_->addAction(saveAct_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(exitAct_);

    editMenu_->addAction(centerAct_);
    editMenu_->addAction(clearAct_);
    legMenu_ = editMenu_->addMenu(window->tr("Leg"));
    legMenu_->addAction(legLAct_);
    legMenu_->addAction(legRAct_);
    editMenu_->addAction(settingsAct_);
    editMenu_->addSeparator();
    editMenu_->addAction(calcAct_);

    helpMenu_->addAction(helpAct_);
    helpMenu_->addSeparator();
    helpMenu_->addAction(aboutAct_);
}

void LayoutWindow::setEnabled(bool value)
{
    saveAct_->setEnabled(value);
    centerAct_->setEnabled(value);
    clearAct_->setEnabled(value);
    calcAct_->setEnabled(value);
}
