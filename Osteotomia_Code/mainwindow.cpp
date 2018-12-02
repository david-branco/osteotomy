#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr(APP_NAME));

    imageWidget_.reset(new CVImageWidget());
    imageWidget_->setFixedSize(MAIN_WINDOW_W, MAIN_WINDOW_H);
    ioProcessor_.reset(new IOProcessor([this](Events event, void* parameters)
    {
        this->eventHandler(event, parameters);
    }));
    layoutWindow_.reset(new LayoutWindow(this,imageWidget_.get(),[this](Events event, void* parameters)
    {
        this->eventHandler(event, parameters);
    }));
    layoutWindow_->setEnabled(false);

    statusLabel_.reset(new QLabel(this));
    statusBar()->addPermanentWidget(statusLabel_.get());

    displayRoi_.reset(new cv::Rect());
    points_.reset(new std::vector<std::shared_ptr<Point>>());
    lastIdx_ = -1;
    leg_ = Leg::LEFT;

    loadSettings("settings.ini");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings(const char* filePath)
{
    settings_.reset(new QSettings(filePath, QSettings::IniFormat));
    settings_->beginGroup("Settings");

    if(settings_->contains("claheTileGridSize") == false) settings_->setValue("claheTileGridSize", SET_CLAHE_TILE_GRID_SIZE);
    if(settings_->contains("claheClipLimit") == false) settings_->setValue("claheClipLimit", SET_CLAHE_CLIP_LIMIT);
    if(settings_->contains("gaussianBlurKernelSize") == false) settings_->setValue("gaussianBlurKernelSize", SET_GAUSSIAN_BLUR_KERNEL_SIZE);
    if(settings_->contains("cannyMinThreshold") == false) settings_->setValue("cannyMinThreshold", SET_CANNY_MIN_THRESHOLD);
    if(settings_->contains("cannyMaxThreshold") == false) settings_->setValue("cannyMaxThreshold", SET_CANNY_MAX_THRESHOLD);
    if(settings_->contains("horizontalRemove") == false) settings_->setValue("horizontalRemove", SET_HORINZONTAL_REMOVE);
    if(settings_->contains("verticalRemove") == false) settings_->setValue("verticalRemove", SET_VERTICAL_REMOVE);
    if(settings_->contains("autoMinAng") == false) settings_->setValue("autoMinAng", SET_AUTO_MIN_ANGLE);
    if(settings_->contains("autoMaxAng") == false) settings_->setValue("autoMaxAng", SET_AUTO_MAX_ANGLE);
    if(settings_->contains("autoAngStep") == false) settings_->setValue("autoAngStep", SET_AUTO_ANG_STEP);
    if(settings_->contains("autoMinLinear") == false) settings_->setValue("autoMinLinear", SET_AUTO_MIN_LINEAR);
    if(settings_->contains("autoMaxLinear") == false) settings_->setValue("autoMaxLinear", SET_AUTO_MAX_LINEAR);
    if(settings_->contains("autoLinearStep") == false) settings_->setValue("autoLinearStep", SET_AUTO_LINEAR_STEP);
    if(settings_->contains("autoLinearSize") == false) settings_->setValue("autoLinearSize", SET_AUTO_LINEAR_SIZE);
    if(settings_->contains("autoScale") == false) settings_->setValue("autoScale", SET_AUTO_SCALE);

    settings_->sync();
}

void MainWindow::initialize(IplImage* image)
{
    loadedImage_.reset(new cv::Mat(cv::cvarrToMat(image)));
    positionAndSize_.reset(new cv::Rect(Utils::getGoodRect(loadedImage_.get(), imageWidget_->width(), imageWidget_->height())));
    setMatToCorrectAspectRatio (loadedImage_.get(), positionAndSize_.get());
    minWidth_ = ZOOM_IN_MAX;
    visibleWidth_ = loadedImage_->cols;
    px_ = loadedImage_->cols/2;
    py_ = loadedImage_->rows/2;
}

void MainWindow::setMatToCorrectAspectRatio (cv::Mat* img, cv::Rect* size)
{
    cv::Rect roi(size->x, size->y, img->cols, img->rows);
    //cv::Scalar color(pow(2,img->depth()*8)-1);
    cv::Scalar color(0);
    cv::Mat image = cv::Mat( size->height, size->width, img->type(), color);
    img->copyTo(image(roi));
    image.copyTo(*img);
}

cv::Mat MainWindow::getImage( cv::Mat& img, float& x, float& y, int width, int target_width, int target_height, cv::Rect* roi)
{
    assert(img.cols / target_width == img.rows / target_height);
    int height = (width * target_height) / target_width;
    int hWidth = width/2;
    int hHeight = height/2;
    //Fix bad points
    x = (x < hWidth ? hWidth : ((x-hWidth)+width > img.cols ? (img.cols-width)+hWidth : x));
    y = (y < hHeight ? hHeight : ((y-hHeight)+height > img.rows ? (img.rows-height)+hHeight : y));
    //new image
    cv::Mat image = cv::Mat( target_height, target_width, img.type());
    roi->x = x-hWidth;
    roi->y = y-hHeight;
    roi->width = width;
    roi->height = height;
    cv::resize(img(*roi), image, image.size());
    return image;
}

void MainWindow::drawGraphics(cv::Mat* image, bool adjustToScreen)
{
    for(auto point : *points_.get())
    {
        cv::Point pt(point->intX(), point->intY());
        if(adjustToScreen)
        {
            pt = worldPointToScreenPoint(pt);
        }
        cv::circle(*image, pt, POINT_RADIUS, cv::Scalar(127, 255, 127), CV_FILLED);
    }
    if(points_->size() == 3)
    {
        cv::Scalar red(0, 0, 255);
        cv::Scalar blue(255, 63, 63);
        cv::Scalar white(255, 255, 255);
        cv::Scalar orange(0, 127, 255);

        std::sort(points_->begin(), points_->end(), [](std::shared_ptr<Point> a, std::shared_ptr<Point> b){ return a->y < b->y; });
        Point* a = points_->at(0).get();
        Point* b = points_->at(1).get();
        Point* c = points_->at(2).get();
        cv::Point sa;
        cv::Point sb;
        cv::Point sc;
        if(adjustToScreen)
        {
            sa = worldPointToScreenPoint(cv::Point(a->intX(), a->intY()));
            sb = worldPointToScreenPoint(cv::Point(b->intX(), b->intY()));
            sc = worldPointToScreenPoint(cv::Point(c->intX(), c->intY()));
        }
        else
        {
            sa = cv::Point(a->intX(), a->intY());
            sb = cv::Point(b->intX(), b->intY());
            sc = cv::Point(c->intX(), c->intY());
        }
        cv::line(*image, sa, sb, red);
        cv::line(*image, sb, sc, red);

        //
        cv::Point v1(a->x-b->x, a->y-b->y);
        cv::Point v2(c->x-b->x, c->y-b->y);
        float dot = v1.x*v2.x + v1.y*v2.y;
        float det = v1.x*v2.y - v1.y*v2.x;
        float rads = std::atan2(det, dot);
        float ang = (rads * 180) / PI;
        float start = std::atan2(v1.y, v1.x);
        start = (start * 180) / PI;
        ellipse( *image, sb, cv::Size(POINT_RADIUS+10,POINT_RADIUS+10), start, 0, ang, blue);

        //
        cv::Point sd(Utils::rotatePoint(sc, sb, (PI-rads)));
        cv::line(*image, sb, sd, orange);

        cv::Point vecB1 ( sb + ((sc - sb) * WEDGE_LOCATION_FACTOR));
        cv::Point vecB0(Utils::rotatePoint(vecB1, sb, -(PI*WEDGE_SIZE_ANGLE)));
        cv::Point vecB2(Utils::rotatePoint(vecB1, sb,  (PI*WEDGE_SIZE_ANGLE)));


        if(ang > 0.0)
        {
            vecB0 = Utils::rotatePoint(vecB0, vecB0 + (vecB2 - vecB0), WEDGE_ANGLE);
            vecB2 = Utils::rotatePoint(vecB2, vecB0 + (vecB2 - vecB0), WEDGE_ANGLE);
            cv::line(*image, vecB0, vecB2, orange);
            cv::Point vecB3(Utils::rotatePoint(vecB0, vecB2, (PI-rads)));
            cv::line(*image, vecB2, vecB3, orange);
        }
        else if(ang < 0.0)
        {
            vecB0 = Utils::rotatePoint(vecB0, vecB2 + (vecB0 - vecB2), -WEDGE_ANGLE);
            vecB2 = Utils::rotatePoint(vecB2, vecB2 + (vecB0 - vecB2), -WEDGE_ANGLE);
            cv::line(*image, vecB0, vecB2, orange);
            cv::Point vecB3(Utils::rotatePoint(vecB2, vecB0, (PI-rads)));
            cv::line(*image, vecB0, vecB3, orange);
        }

        //Text
        if(leg_ == Leg::LEFT)
        {
            ang = -ang;
        }
        const char* type = "Varus Case";
        if(ang < 0.0f)
        {
            ang = - ang;
            type = "Valgus Case";
        }
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 0.5;
        int thickness = 1;
        int baseline=0;
        std::stringstream ss;
        ss << "angle: " << ang;
        std::string text = ss.str();
        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
        cv::Point textOrg(sb.x + POINT_RADIUS+20, sb.y + (textSize.height/2));
        // then put the text itself
        cv::putText(*image, text, textOrg, fontFace, fontScale, white, thickness, 8);
        cv::putText(*image, std::string("o"), textOrg + cv::Point(textSize.width, -textSize.height/2), fontFace, fontScale *(2.0/3.0), white, thickness, 8);
        ss.str(std::string());
        ss << "wedge size: " << (180.0f - ang) << " mm";
        cv::putText(*image, ss.str(), textOrg + cv::Point(0, +textSize.height*1.3f), fontFace, fontScale, white, thickness, 8);
        ss.str(std::string());
        ss << type;
        cv::putText(*image, ss.str(), textOrg + cv::Point(0, -textSize.height*1.3f), fontFace, fontScale, white, thickness, 8);
    }
}

void MainWindow::updateScreenImage()
{
    if(loadedImage_.get() == NULL)
    {
        return;
    }
    cv::Mat displayImage = getImage(*loadedImage_, px_, py_, visibleWidth_, imageWidget_->width(), imageWidget_->height(), displayRoi_.get());
    zoomFactor_ = static_cast<float>(visibleWidth_)/static_cast<float>(imageWidget_->width());
    //convert to 8bits color
    cv::Mat colorImage;
    convertTo8BitColor(displayImage, colorImage);
    // draw points, lines, etc if they exist in a different image
    cv::Mat draws(colorImage.rows, colorImage.cols, colorImage.type(), cv::Scalar::all(0));
    drawGraphics(&draws);
    // sum the 2 images (the dicom image wuth the draws image)
    cv::add(colorImage, draws, displayImage);
    //display the final image
    imageWidget_->showImage(displayImage);

}

void MainWindow::updateStatus()
{
    if(loadedImage_.get() == NULL)
    {
        return;
    }
    std::stringstream ss;
    std::string bp = "<font color=\"blue\">";
    std::string gp = "<font color=\"green\">";
    std::string rp = "<font color=\"red\">";
    std::string s = "</font>";
    ss << gp<<dicomImage_->image->width<<s<< "x" << gp<<dicomImage_->image->height<<s;
    ss << "  Name: " << bp<<dicomImage_->name<<s;
    ss << ", Birthday: " << bp<<dicomImage_->birthday<<s;
    ss << ", Gender: " << bp<<dicomImage_->gender<<s;
    ss << " | " << rp<<(leg_ == Leg::LEFT ? "L" : "R")<<s;
    statusLabel_->setText(ss.str().c_str());
}

void MainWindow::loadImage(const wchar_t* filename)
{
    clearPoints();
    dicomImage_.reset();
    dicomImage_ = DicomLoader().loadImage(filename);
    if(dicomImage_.get() == NULL)
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","It's not possible to load this DICOM.");
        messageBox.show();
        return;
    }
    initialize(dicomImage_->image);
    assert(loadedImage_ != NULL);
    updateScreenImage();
    updateStatus();
    layoutWindow_->setEnabled(true);
}

void MainWindow::convertTo8BitColor(cv::Mat& src, cv::Mat& dst)
{
    cv::Mat img(src);
    //convert to 8bits if necessary
    if(img.depth() == CV_16U)
    {
        img.convertTo(img, CV_8U, FACTOR_16TO8);
    }
    //convert to a color image (cv::Mat with 3 channels)
    std::vector<cv::Mat> channels(3);
    channels.at(0) = img; //for blue channel
    channels.at(1) = img; //for green channel
    channels.at(2) = img; //for red channel
    //merge the 3 channels in one single cv::Mat
    cv::merge(channels, dst);
}

void MainWindow::saveImage(cv::Mat& image, const char* filename)
{
    std::vector<int> params;
    params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    params.push_back(0);
    cv::Mat colorImage;
    convertTo8BitColor(image, colorImage);
    drawGraphics(&colorImage, false);
    cv::imwrite(filename, colorImage, params);
}

void MainWindow::wheelEvent(QWheelEvent * event){ioProcessor_->wheelEvent(event);}
void MainWindow::keyPressEvent(QKeyEvent *event){ioProcessor_->keyPressEvent(event);}
void MainWindow::mouseMoveEvent(QMouseEvent *event){ioProcessor_->mouseMoveEvent(event);}
void MainWindow::mousePressEvent(QMouseEvent *event){ioProcessor_->mousePressEvent(event);}
void MainWindow::mouseReleaseEvent(QMouseEvent *event){ioProcessor_->mouseReleaseEvent(event);}

void MainWindow::resizeEvent(QResizeEvent*)
{
    if(dicomImage_)
    {
        std::vector<Point> points;
        while(points_->size() > 0)
        {
            Point p(*points_->at(0));
            points_->erase(points_->begin());
            p.x -= positionAndSize_->x;
            p.y -= positionAndSize_->y;
            points.push_back(p);
        }

        float fx = static_cast<float>(px_) / static_cast<float>(positionAndSize_->width);
        float fy = static_cast<float>(py_) / static_cast<float>(positionAndSize_->height);
        float fw = static_cast<float>(visibleWidth_) / static_cast<float>(positionAndSize_->width);

        QSize availableSize = this->centralWidget()->size();
        int w = availableSize.width() - availableSize.width()%2;
        int h = availableSize.height() - availableSize.height()%2;
        imageWidget_->setFixedSize(w, h);

        initialize(dicomImage_->image);

        while(points.size() > 0)
        {
            Point p(points.at(0));
            points.erase(points.begin());
            p.x += positionAndSize_->x;
            p.y += positionAndSize_->y;
            addPoint(p.x, p.y);
        }

        px_ = static_cast<int>(fx * positionAndSize_->width);
        py_ = static_cast<int>(fy * positionAndSize_->height);
        visibleWidth_ = static_cast<int>(fw * positionAndSize_->width);

        updateScreenImage();
    }
}

cv::Point MainWindow::screenPointToWorldPoint (cv::Point point)
{
    //ajust position to be inside the ImageWidget
    QPoint windowPoint = imageWidget_->mapTo(imageWidget_->window(), imageWidget_->pos());
    point.x -= windowPoint.x();
    point.y -= windowPoint.y();
    //adjust to zoom
    point *= zoomFactor_;
    //ajust to Roi
    point += cv::Point(displayRoi_->x, displayRoi_->y);
    return point;
}

cv::Point MainWindow::worldPointToScreenPoint (cv::Point point)
{
    point.x -= displayRoi_->x;
    point.y -= displayRoi_->y;
    point /= zoomFactor_;
    return point;
}

void MainWindow::clearPoints()
{
    points_->clear();
}

int MainWindow::mouseOverPoint (cv::Point mousePoint)
{
    mousePoint = screenPointToWorldPoint (mousePoint);
    //find points below the mouse pointer
    int idx = 0;
    for(auto point : *points_.get())
    {
        double radius = cv::norm(mousePoint-point->toCv());
        if(radius <= POINT_RADIUS*zoomFactor_)
        {
            return idx;
        }
        ++idx;
    }
    return -1;
}

Vec2iPair
MainWindow::getAutoPoint( cv::Mat& bone, cv::Mat& image, BoneDetector::Bone boneName, double& factor)
{
    cv::Vec2i pointA, pointB;
    float ang;
    float ratio;
    BoneDetector boneDetector;
    boneDetector.setAngularPars(
        settings_->value("autoMinAng").toDouble(),
        settings_->value("autoMaxAng").toDouble(),
        settings_->value("autoAngStep").toInt());
    boneDetector.setLinearPars(
        settings_->value("autoMinLinear").toInt(),
        settings_->value("autoMaxLinear").toInt(),
        settings_->value("autoLinearStep").toInt(),
        settings_->value("autoLinearSize").toInt());
    boneDetector.createRtable(bone);
    boneDetector.accumulate(image);
    factor = boneDetector.bestCandidate( boneName, pointA, pointB, ang, ratio);
    return std::make_pair( pointA, pointB);
}

void MainWindow::automatic()
{
    if (loadedImage_.get() == NULL)
    {
        return;
    }

    clearPoints();

    int scale = settings_->value("autoScale").toInt();
    assert(scale > 0);

    cv::Mat img;
    cv::Mat temp(cv::cvarrToMat(dicomImage_->image));
    cv::resize( temp, img, cv::Size(temp.cols / scale, temp.rows / scale));
    if(img.depth() == CV_16U)
    {
        img.convertTo(img, CV_8U, FACTOR_16TO8);
    }

    EdgeDetector edgeDetector;
    edgeDetector.blurSize(settings_->value("gaussianBlurKernelSize").toInt());
    edgeDetector.claheClipLimit(settings_->value("claheClipLimit").toInt());
    edgeDetector.claheTilesGridSize(settings_->value("claheTileGridSize").toInt());
    edgeDetector.cannyThreshold1(settings_->value("cannyMinThreshold").toInt());
    edgeDetector.cannyThreshold2(settings_->value("cannyMaxThreshold").toInt());
    edgeDetector.horizontalSize(settings_->value("horizontalRemove").toInt());
    edgeDetector.verticalSize(settings_->value("verticalRemove").toInt());
    edgeDetector.process(img, img);

    cv::Mat boneA, boneB;
    bool loaded = false;
    loaded = Utils::loadImageFromResource(boneA, ":/images/Femur.png");
    assert(loaded == true);
    loaded = Utils::loadImageFromResource(boneB, ":/images/Tibia.png");
    assert(loaded == true);

    BoneDetector::Bone boneNameA = BoneDetector::RIGHT_FEMUR;
    BoneDetector::Bone boneNameB = BoneDetector::RIGHT_TIBIA;
    if(leg_ == LEFT)
    {
        cv::flip(boneA, boneA, 1);
        cv::flip(boneB, boneB, 1);
        boneNameA = BoneDetector::LEFT_FEMUR;
        boneNameB = BoneDetector::LEFT_TIBIA;
    }
    double factorA = 1.0, factorB = 1.0;

    Vec2iPair pointsA(getAutoPoint( boneA, img, boneNameA, factorA));
    Vec2iPair pointsB(getAutoPoint( boneB, img, boneNameB, factorB));

    if(factorA < WARNING_LIMIT || factorB < WARNING_LIMIT)
    {
        int value = WARNING_LIMIT*100;
        QMessageBox::warning(
            this,
            tr(APP_NAME),
            tr("The detection ratio is below %n%", "", value),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return;
    }

    addPoint( positionAndSize_->x + pointsA.first[0]*scale, positionAndSize_->y + pointsA.first[1]*scale);
    addPoint( positionAndSize_->x + pointsB.first[0]*scale, positionAndSize_->y + pointsB.first[1]*scale);
    addPoint( positionAndSize_->x + pointsB.second[0]*scale, positionAndSize_->y + pointsB.second[1]*scale);

    updateScreenImage();
}

void MainWindow::addPoint( int x, int y)
{
    points_->push_back(std::make_shared<Point>(Point( x, y)));
}

void MainWindow::centerImage()
{
    if(loadedImage_.get () == NULL)
    {
        return;
    }
    visibleWidth_ = loadedImage_->cols;
    px_ = loadedImage_->cols/2;
    py_ = loadedImage_->rows/2;
}

void MainWindow::eventHandler(Events event, void* parameters)
{
    switch(event)
    {
        case ZOOM_IN:
        {
            if (loadedImage_.get() == NULL)
            {
                return;
            }

            int step = 0;
            unsigned int numberOfZoom = (parameters != NULL ? *static_cast<int*>(parameters) : 1);
            for(; numberOfZoom !=0 ; --numberOfZoom)
            {
                step += ((visibleWidth_-step) - ZOOM_STEP >= minWidth_ ? ZOOM_STEP : (visibleWidth_-step) - minWidth_);
            }
            if(step > 0)
            {
                visibleWidth_ -= step;
                updateScreenImage();
            }
        }
        break;

        case ZOOM_OUT:
        {
            if (loadedImage_.get() == NULL)
            {
                return;
            }

            int step = 0;
            unsigned int numberOfZoom = (parameters != NULL ? *static_cast<int*>(parameters) : 1);
            for(; numberOfZoom !=0 ; --numberOfZoom)
            {
                step += ((visibleWidth_+step) + ZOOM_STEP <= loadedImage_->cols ? ZOOM_STEP : loadedImage_->cols - (visibleWidth_+step));
            }
            if(step > 0)
            {
                visibleWidth_ += step;
                updateScreenImage();
            }
        }
        break;

        case MOVE_UP:
        {
            py_ -= MOVE_STEP;
            updateScreenImage();
        }
        break;

        case MOVE_DOWN:
        {
            py_ += MOVE_STEP;
            updateScreenImage();
        }
        break;

        case MOVE_LEFT:
        {
            px_ -= MOVE_STEP;
            updateScreenImage();
        }
        break;

        case MOVE_RIGHT:
        {
            px_ += MOVE_STEP;
            updateScreenImage();
        }
        break;

        case MOVE:
        {
            if(parameters != NULL)
            {
                std::pair<std::pair<int,int>,std::pair<int,int>> data = *static_cast<std::pair<std::pair<int,int>,std::pair<int,int>>*>(parameters);
                std::pair<int,int> delta = data.first;
                std::pair<int,int> mousePosition= data.second;

                float x = delta.first * zoomFactor_;
                float y = delta.second * zoomFactor_;

                cv::Point mousePoint(mousePosition.first, mousePosition.second);
                int idx = mouseOverPoint(mousePoint);
                idx = (idx == -1 ? lastIdx_ : idx);
                if(idx != -1)
                {
                    Point* p = points_->at(idx).get();
                    p->x += x;
                    p->y += y;
                    lastIdx_ = idx;
                }
                else
                {
                    px_ -= x;
                    py_ -= y;
                }
                updateScreenImage();
            }
        }
        break;

        case MOUSE_LEFT_RELEASE:
        {
            if(lastIdx_ == -1 && points_->size() < 3 && parameters != NULL)
            {
                std::pair<int,int> data = *static_cast<std::pair<int,int>*>(parameters);
                cv::Point point = screenPointToWorldPoint(cv::Point(data.first, data.second));
                if(point.x >= 0 && point.y >= 0)
                {
                    addPoint(point.x,point.y);
                    updateScreenImage();
                }
            }
            lastIdx_ = -1;
        }
        break;

        case MOUSE_RIGHT_RELEASE:
        {
            if(parameters != NULL)
            {
                std::pair<int,int> data = *static_cast<std::pair<int,int>*>(parameters);
                cv::Point mousePoint = cv::Point(data.first, data.second);
                int idx = mouseOverPoint(mousePoint);
                if(idx != -1)
                {
                    points_->erase(points_->begin() + idx);
                    updateScreenImage();
                }
            }
        }
        break;

        case CENTER:
        {
            centerImage();
            updateScreenImage();
        }
        break;

        case OPEN:
        {
            const wchar_t* filename = const_cast<const wchar_t*>(static_cast<wchar_t*>(parameters));
            loadImage(filename);
        }
        break;

        case SAVE_AS:
        {
            const char* filename = const_cast<const char*>(static_cast<char*>(parameters));
            if(loadedImage_ != NULL)
            {
                saveImage(*loadedImage_, filename);
            }
        }
        break;

        case CLEAR_POINTS:
        {
            clearPoints();
            updateScreenImage();
        }
        break;

        case LEG_LEFT:
        {
            if(leg_ != Leg::LEFT)
            {
                leg_ = Leg::LEFT;
                updateStatus();
                clearPoints();
                updateScreenImage();
            }
        }
        break;
        case LEG_RIGHT:
        {
            if(leg_ != Leg::RIGHT)
            {
                leg_ = Leg::RIGHT;
                updateStatus();
                clearPoints();
                updateScreenImage();
            }
        }
        break;

        case HELP:
        {
            WindowDialog helpWindow(this, "<b>Key - function</b><p><b>Esc</b> - Close<p><b>Space</b> - Center image<p><b>c</b> - Clear points<p><b>wasd/directional</b> - Move<p><b>+-</b> - zoom<p><b>Left Click</b> - Add point<p><b>Right Click</b> - Remove point<p><b>Mouse click and drag</b> - Pan<p><b>Mouse click and drag over point</b> - Move point<p><b>Mouse Scroll</b> - Zoom");
            helpWindow.exec();
        }
        break;

        case ABOUT:
        {
            WindowDialog aboutWindow(this, "David Branco [PG27720]<p>Miguel Ribeiro [A57767]<p>Tiago Santos [A64381]");
            aboutWindow.exec();
        }
        break;

        case AUTOMATIC:
        {
            automatic();
        }
        break;

        case SETTINGS:
        {
            WindowDialog settingsWindow(this, *settings_);
            settingsWindow.exec();
        }
        break;
    }
}
