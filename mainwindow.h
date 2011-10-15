#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QDialog>

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
    QLabel* photoLabel;

    QPushButton* loadBtn;
    QPushButton* scannBtn;
    QPushButton* recognizeBtn;

    QTextEdit* logTextEd;

    void CreateRow(QString comment, QLineEdit*& edit);
    void WriteLog(QString msg);

public slots:
    void Recognize();
    void LoadFromFile();
    void LoadFromScanner();
};

#endif // MAINWINDOW_H
