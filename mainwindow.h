#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QDialog>
#include <QThread>
#include <QMutex>
#include <QEvent>
#include <QSettings>

#ifndef NO_SCANYAPY
#include "ScProxy.h"
#endif

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
class QProgressBar;
class QStatusBar;
class QTextEdit;
class QVBoxLayout;
QT_END_NAMESPACE


class ScanyThread : public QThread
{
    Q_OBJECT

public:
    virtual void run();
    ScanyThread(QObject * parent = 0);
    ~ScanyThread() { CloseScanyApi(); }

private:
#ifndef NO_SCANYAPY
    ScInstance* scanyInstance;
    ScPackage* scanyPackage;
    ScScanDevice* scanyDevice;
#endif
    QMutex scanyInitMutex;
    QMutex scanyRecognizeMutex;

    int progressSteps;
    int currentStep;

    bool InitScanyApi();
    bool CloseScanyApi();
    void SetProgressSteps(int n) { progressSteps = n; currentStep = 0; }
    void NextProgressStep(QString msg);
    void WriteLog(QString msg);

public slots:
    void Recognize();

signals:
    void ProgressChanged();
    void DataProcessed();
    void Initialized();
    void MessageSent();
};

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    ScanyThread* scanyThread;

    QVBoxLayout* mainLayout;
    QFormLayout* formLayout;
    QProgressBar* progressBar;
    QStatusBar* statusBar;

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

    QPushButton* recognizeBtn;
    QTextEdit* logTextEd;

    QSettings settings;
    QString userName;
    QString password;
    QString databaseName;
    QString hostName;

    void CreateRow(QString comment, QLineEdit*& edit);
    void LoadDefaultImage();
    void ReadSettings();
    void WriteSettings();
    bool ConnectDatabase();

public slots:   
    void ChangeProgress();
    void ReadData();
    void ShowMessages();
};

#endif // MAINWINDOW_H
