#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    QFormLayout* mainLayout;

    QLineEdit* sernameEd;
    QLineEdit* nameEd;
    QLineEdit* middlenameEd;
    QLineEdit* genderEd;
    QLineEdit* serialNumEd;
    QLineEdit* birthDateEd;
    QLineEdit* birthPlaceEd;
    QLineEdit* issueData;
    QLineEdit* givenByUnit;
    QLineEdit* gibenByCode;

    void CreateRow(QString comment, QLineEdit*& edit);
};

#endif // MAINWINDOW_H
