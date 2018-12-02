#include "windowdialog.h"

WindowDialog::WindowDialog(QMainWindow *parent, const char* message) :
    QDialog(parent)
{
    this->resize( 360, 280 );
    mainLayout_ = new QBoxLayout( QBoxLayout::TopToBottom, this );
    this->setLayout(mainLayout_);

    QLabel* label = new QLabel( message, this);
    label->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(label);
}

void
WindowDialog::addLabelSpinBox(const char* labelText, double min, double max, double defaultValue, double increment, QSettings& settings, const char* key, QVBoxLayout *vbox,  std::vector<std::pair<QDoubleSpinBox*,double>>& vector)
{
    QLabel *label = new QLabel(tr(labelText).arg(min).arg(max).arg(defaultValue));
    vbox->addWidget(label);
    QDoubleSpinBox *spinBox = new QDoubleSpinBox;
    vector.push_back(std::pair<QDoubleSpinBox*,double>(spinBox, defaultValue));
    spinBox->setRange(min, max);
    spinBox->setSingleStep(increment);
    spinBox->setValue(settings.value(key).toDouble());
    vbox->addWidget(spinBox);
    void (QDoubleSpinBox:: *signal)(double) = &QDoubleSpinBox::valueChanged;
    connect(spinBox, signal, [spinBox, &settings, key]()
    {
        QString newValue(QString::number(spinBox->value()));
        settings.setValue(key, newValue);
        settings.sync();
    });
}

void
WindowDialog::addLabelSpinBox(const char* labelText, int min, int max, int defaultValue, int increment, QSettings& settings, const char* key, QVBoxLayout *vbox, std::vector<std::pair<QSpinBox*,int>>& vector, bool forceOdd)
{
    QLabel *label = new QLabel(tr(labelText).arg(min).arg(max).arg(defaultValue));
    vbox->addWidget(label);
    QSpinBox *spinBox = new QSpinBox;
    vector.push_back(std::pair<QSpinBox*,int>(spinBox, defaultValue));
    spinBox->setRange(min, max);
    spinBox->setSingleStep(increment);
    spinBox->setValue(settings.value(key).toInt());
    vbox->addWidget(spinBox);
    void (QSpinBox:: *signal)(int) = &QSpinBox::valueChanged;
    connect(spinBox, signal, [spinBox, &settings, key, forceOdd]()
    {
        int value = spinBox->value();
        if(forceOdd && value > 0 && value%2 == 0)
        {
              spinBox->setValue(value+1);
        }
        QString newValue(QString::number(spinBox->value()));
        settings.setValue(key, newValue);
        settings.sync();
    });
}

WindowDialog::WindowDialog(QMainWindow *parent, QSettings& settings) :
    QDialog(parent)
{
    this->resize( 360, 280 );
    mainLayout_ = new QBoxLayout( QBoxLayout::TopToBottom, this );
    this->setLayout(mainLayout_);

    QGroupBox *groupBoxA = new QGroupBox(tr("Phase 1 - Downscale"));
    QGroupBox *groupBoxB = new QGroupBox(tr("Phase 2 - Improve Contrast"));
    QGroupBox *groupBoxC = new QGroupBox(tr("Phase 3 - Remove Grid"));
    QGroupBox *groupBoxD = new QGroupBox(tr("Phase 4 - Reduce details"));
    QGroupBox *groupBoxE = new QGroupBox(tr("Phase 5 - Segmentation"));
    QGroupBox *groupBoxF = new QGroupBox(tr("Phase 6 - Bone detection"));
    QVBoxLayout *vboxA = new QVBoxLayout;
    QVBoxLayout *vboxB = new QVBoxLayout;
    QVBoxLayout *vboxC = new QVBoxLayout;
    QVBoxLayout *vboxD = new QVBoxLayout;
    QVBoxLayout *vboxE = new QVBoxLayout;
    QVBoxLayout *vboxF = new QVBoxLayout;
    groupBoxA->setLayout(vboxA);
    groupBoxB->setLayout(vboxB);
    groupBoxC->setLayout(vboxC);
    groupBoxD->setLayout(vboxD);
    groupBoxE->setLayout(vboxE);
    groupBoxF->setLayout(vboxF);
    QVBoxLayout *vboxL = new QVBoxLayout;
    QVBoxLayout *vboxR = new QVBoxLayout;
    vboxL->addWidget(groupBoxA);
    vboxL->addWidget(groupBoxB);
    vboxL->addWidget(groupBoxC);
    vboxL->addWidget(groupBoxD);
    vboxR->addWidget(groupBoxE);
    vboxR->addWidget(groupBoxF);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addLayout(vboxL);
    hbox->addLayout(vboxR);

    std::vector<std::pair<QSpinBox*,int>> vecA;
    std::vector<std::pair<QDoubleSpinBox*,double>> vecB;
    addLabelSpinBox("Automatic scale. Enter a value between %1 and %2. Default is %3.",           1,   32, SET_AUTO_SCALE,                  1, settings, "autoScale",              vboxA, vecA);
    addLabelSpinBox("CLAHE tile grid size. Enter a value between %1 and %2. Default is %3.",      0,  255, SET_CLAHE_TILE_GRID_SIZE,        1, settings, "claheTileGridSize",      vboxB, vecA);
    addLabelSpinBox("CLAHE clip limit. Enter a value between %1 and %2. Default is %3.",          0,   15, SET_CLAHE_CLIP_LIMIT,            1, settings, "claheClipLimit",         vboxB, vecA);
    addLabelSpinBox("Horizontal lines remove. Enter a value between %1 and %2. Default is %3.",   0,   15, SET_HORINZONTAL_REMOVE,          1, settings, "horizontalRemove",       vboxC, vecA);
    addLabelSpinBox("Vertical lines remove. Enter a value between %1 and %2. Default is %3.",     0,   15, SET_VERTICAL_REMOVE,             1, settings, "verticalRemove",         vboxC, vecA);
    addLabelSpinBox("GaussianBlur kernel size. Enter a value between %1 and %2. Default is %3.",  0,  255, SET_GAUSSIAN_BLUR_KERNEL_SIZE,   2, settings, "gaussianBlurKernelSize", vboxD, vecA, true);
    addLabelSpinBox("Canny minimum Threshold. Enter a value between %1 and %2. Default is %3.",   0,  255, SET_CANNY_MIN_THRESHOLD,         1, settings, "cannyMinThreshold",      vboxE, vecA);
    addLabelSpinBox("Canny maximum Threshold. Enter a value between %1 and %2. Default is %3.",   0,  255, SET_CANNY_MAX_THRESHOLD,         1, settings, "cannyMaxThreshold",      vboxE, vecA);
    addLabelSpinBox("Automatic minimum angle. Enter a value between %1 and %2. Default is %3.", -PI,   PI, SET_AUTO_MIN_ANGLE,            0.1, settings, "autoMinAng",             vboxF, vecB);
    addLabelSpinBox("Automatic maximum angle. Enter a value between %1 and %2. Default is %3.", -PI,   PI, SET_AUTO_MAX_ANGLE,            0.1, settings, "autoMaxAng",             vboxF, vecB);
    addLabelSpinBox("Automatic angle steps. Enter a value between %1 and %2. Default is %3.",     0,  360, SET_AUTO_ANG_STEP,               1, settings, "autoAngStep",            vboxF, vecA);
    addLabelSpinBox("Automatic minimum size. Enter a value between %1 and %2. Default is %3.",    0, 9999, SET_AUTO_MIN_LINEAR,             1, settings, "autoMinLinear",          vboxF, vecA);
    addLabelSpinBox("Automatic maximum size. Enter a value between %1 and %2. Default is %3.",    0, 9999, SET_AUTO_MAX_LINEAR,             1, settings, "autoMaxLinear",          vboxF, vecA);
    addLabelSpinBox("Automatic size step. Enter a value between %1 and %2. Default is %3.",       0, 9999, SET_AUTO_LINEAR_STEP,            1, settings, "autoLinearStep",         vboxF, vecA);
    addLabelSpinBox("Automatic linear size. Enter a value between %1 and %2. Default is %3.",     0, 9999, SET_AUTO_LINEAR_SIZE,            1, settings, "autoLinearSize",         vboxF, vecA);

    QPushButton *button = new QPushButton("&Reset All");
    vboxL->addWidget(button);
    connect(button, &QPushButton::clicked, [vecA, vecB]()
    {
        for(size_t i = 0; i < vecA.size(); i++)
        {
            vecA[i].first->setValue(vecA[i].second);
        }
        for(size_t i = 0; i < vecB.size(); i++)
        {
            vecB[i].first->setValue(vecB[i].second);
        }
    });

    mainLayout_->addLayout(hbox);
}

WindowDialog::~WindowDialog()
{
    delete mainLayout_;
}
