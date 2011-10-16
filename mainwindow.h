#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QDialog>
#include "ScProxy.h"

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
class QVBoxLayout;
QT_END_NAMESPACE

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    QVBoxLayout* mainLayout;
    QFormLayout* formLayout;

    QLineEdit* surnameEd;
    QLineEdit* nameEd;
    QLineEdit* secondNameEd;
    QLineEdit* sexEd;
    QLineEdit* serialEd;
    QLineEdit* numberEd;
    QLineEdit* birthDateEd;
    QLineEdit* birthPlaceEd;
    QLineEdit* issueDate;
    QLineEdit* givenByUnit;
    QLineEdit* givenByCode;
    QLabel* photoLabel;

    QPushButton* loadBtn;
    QPushButton* scannBtn;
    QPushButton* recognizeBtn;

    QTextEdit* logTextEd;

    ScInstance* scanyInstance;
    ScPackage* scanyPackage;
    ScScanDevice* scanyDevice;

    void CreateRow(QString comment, QLineEdit*& edit);
    void WriteLog(QString msg);
    bool InitScanyApi();
    bool CloseScanyApi();

public slots:
    void Recognize();
};

#endif // MAINWINDOW_H
