#ifndef WINDOWDIALOG_H
#define WINDOWDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

#include <QValidator>
#include <QLineEdit>
#include <QSpinBox>
#include <QSettings>
#include <QMainWindow>
#include <QGroupBox>
#include <QPushButton>

#include "config.h"

class WindowDialog : public QDialog
{
private:
    QBoxLayout* mainLayout_;
    void addLabelSpinBox(const char* labelText,    int min,    int max,    int defaultValue,    int increment, QSettings& settings, const char* key, QVBoxLayout *vbox,          std::vector<std::pair<QSpinBox*,int>>& vector, bool forceOdd = false);
    void addLabelSpinBox(const char* labelText, double min, double max, double defaultValue, double increment, QSettings& settings, const char* key, QVBoxLayout *vbox, std::vector<std::pair<QDoubleSpinBox*,double>>& vector);
public:
    WindowDialog(QMainWindow* window, const char* message);
    WindowDialog(QMainWindow* window, QSettings& settings);
    ~WindowDialog();
};

#endif // WINDOWDIALOG_H
