#include "mainwindow.h"
#include <QtGui>
#include <QMutex>
#include <stdio.h>


namespace Message {
    QStringList strList;
    QMutex mutex;
}

namespace Progress {
    int value;
    QString message;
    QMutex mutex;
}

namespace Data {
    QString surnameEd;
    QString nameEd;
    QString secondNameEd;
    QString sexEd;
    QString birthDateEd;
    QString birthPlaceEd;
    QString serialEd;
    QString numberEd;
    QString givenByUnit;
    QString issueDate;
    QString givenByCode;
    QString photoFile;
    QMutex mutex;
}

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      mainLayout(new QVBoxLayout),
      formLayout(new QFormLayout),
      logTextEd(new QTextEdit)
{
    logTextEd->setReadOnly(1);

    QGroupBox* groupBox = new QGroupBox();
    {
        photoLabel = new QLabel;
        // TODO: load dummy photo from resources
        QPixmap pix(100, 100);
        pix.fill(Qt::transparent);
        photoLabel->setPixmap(pix);
        formLayout->addRow(tr("����"), photoLabel);
        mainLayout->addWidget(groupBox);
    }
    groupBox->setLayout(formLayout);
    CreateRow(tr("�������").toAscii(), surnameEd);
    CreateRow(tr("���"), nameEd);
    CreateRow(tr("��������"), secondNameEd);
    CreateRow(tr("���"), sexEd);
    CreateRow(tr("�����"), serialEd);
    CreateRow(tr("�����"), numberEd);
    CreateRow(tr("���� ��������"), birthDateEd);
    CreateRow(tr("����� ��������"), birthPlaceEd);
    CreateRow(tr("���� ��������� ��������"), issueDate);
    CreateRow(tr("������ ��������������"), givenByUnit);
    CreateRow(tr("��� �������������"), givenByCode);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    recognizeBtn = new QPushButton(tr("����������"));    
    btnLayout->addWidget(recognizeBtn);
    mainLayout->addLayout(btnLayout);

    mainLayout->addWidget(logTextEd);

    statusBar = new QStatusBar(this);
    statusBar->showMessage(tr("���������� ��������"));
    progressBar = new QProgressBar;
    statusBar->addPermanentWidget(progressBar);
    mainLayout->addWidget(statusBar);
    //progressLabel = new QLabel(tr("���������� ��������"));
    //QPalette plt;
    //plt.setColor(QPalette::WindowText, Qt::darkGray);
    //progressLabel->setPalette(plt);
    //prl->addWidget(progressLabel);
    //prl->addWidget(progressBar);
    //progressBox->setLayout(prl);
    //mainLayout->addWidget(progressBox);

    mainLayout->setMargin(0);
    setLayout(mainLayout);
    setWindowTitle(tr("�������� �������"));

    scanyThread = new ScanyThread;
    connect(recognizeBtn, SIGNAL(clicked()), scanyThread, SLOT(Recognize()), Qt::QueuedConnection);
    connect(scanyThread, SIGNAL(ProgressChanged()), this, SLOT(ChangeProgress()));
    connect(scanyThread, SIGNAL(DataProcessed()), this, SLOT(ReadData()));
    connect(scanyThread, SIGNAL(MessageSent()), this, SLOT(ShowMessages()));
    scanyThread->start();
}

MainWindow::~MainWindow()
{ 
}

void MainWindow::CreateRow(QString comment, QLineEdit*& edit)
{
    edit = new QLineEdit;
    formLayout->addRow(comment, edit);
}

void MainWindow::ShowMessages()
{
    QStringList list;
    {
        QMutexLocker lock(&Message::mutex);
        list =  Message::strList;
        Message::strList.clear();
    }
    QString str;
    foreach (str, list) {
        logTextEd->setPlainText(QString("%1\n%2").arg(str)
                                                 .arg(logTextEd->toPlainText()));
    }

}

void MainWindow::ChangeProgress()
{
    QMutexLocker lock(&Progress::mutex);

    progressBar->setValue(Progress::value);
    //progressLabel->setText(Progress::message);
    statusBar->showMessage(Progress::message);
    statusBar->showMessage(QString("%1").arg(Progress::value));
}

void MainWindow::ReadData()
{
    QMutexLocker lock(&Data::mutex);

    surnameEd->setText(Data::surnameEd);
    nameEd->setText(Data::nameEd);
    secondNameEd->setText(Data::secondNameEd);
    sexEd->setText(Data::sexEd);
    birthDateEd->setText(Data::birthDateEd);
    birthPlaceEd->setText(Data::birthPlaceEd);
    serialEd->setText(Data::serialEd);
    numberEd->setText(Data::numberEd);
    givenByUnit->setText(Data::givenByUnit);
    issueDate->setText(Data::issueDate);
    givenByCode->setText(Data::givenByCode);
    {
        QPixmap pix;
        pix.load(Data::photoFile);
        pix = pix.scaled(QSize(100, 100), Qt::KeepAspectRatio);
        QBuffer buff;
        pix.save(&buff);
        photoLabel->setPixmap(pix);
    }
}

void ScanyThread::run()
{
    InitScanyApi();
    exec();
}

void ScanyThread::NextProgressStep(QString msg)
{
    {
        QMutexLocker lock(&Progress::mutex);
        Progress::value = 100*(++currentStep) / progressSteps;
        Progress::message = msg;
    }
    emit ProgressChanged();
}

void ScanyThread::WriteLog(QString msg)
{
    {
        QMutexLocker lock(&Message::mutex);
        Message::strList << msg;
    }
    emit MessageSent();
}

#define CF_ROOT		"C:\\Program Files\\Cognitive\\ScanifyAPI\\"

void ScanyThread::Recognize()
{
    sleep(400);

    if (!scanyRecognizeMutex.tryLock()) {
        // TODO: tell user that he is not right
        return;
    }
    QMutexLocker lock(&scanyRecognizeMutex);
    scanyRecognizeMutex.unlock();

#ifdef NO_SCANYAPY

    SetProgressSteps(5);
    NextProgressStep("�������� ������ ������");
    sleep(1);
    NextProgressStep("������������� ���������� ������������");
    sleep(1);
    NextProgressStep("������������");
    sleep(1);
    NextProgressStep("�������������");
    {
        QMutexLocker lock(&Data::mutex);

        Data::surnameEd = "���";
        Data::nameEd = "�������";
        Data::secondNameEd = "��������";
        /*
        Data::sexEd = ScPackageGetFieldValue(sdh, "PP_Sex");
        Data::birthDateEd = ScPackageGetFieldValue(sdh, "PP_BirthDate");
        Data::birthPlaceEd = ScPackageGetFieldValue(sdh, "PP_BirthPlace");
        Data::serialEd = ScPackageGetFieldValue(sdh, "PP_Ser2");
        Data::numberEd = ScPackageGetFieldValue(sdh, "PP_Num2");
        Data::givenByUnit = ScPackageGetFieldValue(sdh, "PP_Kem");
        Data::issueDate = ScPackageGetFieldValue(sdh, "PP_Date");
        Data::givenByCode = ScPackageGetFieldValue(sdh, "PP_Podr");
        Data::photoFile = QString("%1\\%2").arg(packInfo.szName)
        */
    }
    NextProgressStep("���������");

#else

    if (!InitScanyApi()) return;

    // 5. ���� �������� � ����
    WriteLog("\n----------------- ���� �������� �� � ������������ ����");

    // 3. �������� ������ ������
    scanyPackage = ScPackageCreate(scanyInstance);
    if (!scanyPackage) {
        WriteLog("�� ���� �������� ����� ScanyApi");
        WriteLog(QString("\n!������! %1\n").arg(ScGetErrorMessage(scanyInstance)));
        return;
    }

    WriteLog("����� ������");

    int cnt = ScScannerGetTWAINSourceCount(scanyInstance);
    for (int i = 0; i < cnt; ++i)
    {
        const char* c = ScScannerGetTWAINSourceName(scanyInstance, i);
        if (c) WriteLog(c);
    }

    if (!scanyDevice) {
        scanyDevice = ScScannerOpen(scanyInstance, SC_DEV_SCANNER, NULL);
        ScScannerSetConfiguration(scanyDevice, "ScanPassportAndDriveLic");
    }

    // 5.1 ������� ������� ��������� = ������� � �����
    // ��������! ���� �� �������� ����������� ��������������
    // ������� 2� ����������, ��������� ��������� ������������
    // �������������� ���������� � .ini-�����, �� ����
    // ������������ ��������� � ��� ������������� ��� ����������
    // ������������ ���������� �� ������� �����������.
    // ��. [ScanPassportAndDriveLic]/Zone1, Zone2 � �.�

    // 5.2 ������������ ����������� � �����
    WriteLog("��� ������������...");
    ScImage* pImg = ScPackageScan(scanyPackage, scanyDevice, NULL);
    WriteLog("\t���������");
    if (!pImg) {
            WriteLog("������������� 0 �����������!");
    }

    ScScannerClose(scanyDevice);
    scanyDevice = NULL;

    // 5.3 ������������� ����������� � ������
    // (������������ ������ ����� ����������� �����������)
    WriteLog("��� �������������...");
    if (!ScPackageRecog(scanyPackage)){
        WriteLog("������� ������������� ���������� ����������� �������!");
        return;
    }
    WriteLog("\t���������");

    // (5.4) ������ ����������� (��������) ����� � ��������������� � ��������� ����������
    if (pImg){
        if (!ScImageResample(pImg, 120, 120)) {
            WriteLog("�� ���� ����� �����������");
        } else {
            ScExportImageOptions exp = { SC_SAVE_JPG, 90, 0};
            if (!ScImageExport(pImg, "./Passport.jpg", &exp)) {
                WriteLog("�� ���� �������������� �����������");
            }
        }
    }

    // 7.A ������ ����������� ����� ������� ��������� (��������)
    ScDocument *sdh = ScPackageGetFirstDoc(scanyPackage);
    if (!sdh) {
        WriteLog("�� ���� �������� �������� �� �����");
        return;
    }

    WriteLog(QString("�������:   %1").arg(ScPackageGetFieldValue(sdh, "PP_SurName")));
    WriteLog(QString("���:       %1").arg(ScPackageGetFieldValue(sdh, "PP_Name")));
    WriteLog(QString("��������:  %1").arg(ScPackageGetFieldValue(sdh, "PP_SecName")));
    WriteLog(QString("���:  %1").arg(ScPackageGetFieldValue(sdh, "PP_Sex")));
    WriteLog(QString("���� ��������:  %1").arg(ScPackageGetFieldValue(sdh, "PP_BirthDate")));
    WriteLog(QString("���� ��������:  %1").arg(ScPackageGetFieldValue(sdh, "PP_BirthPlace")));
    WriteLog(QString("�����:  %1").arg(ScPackageGetFieldValue(sdh, "PP_Ser2")));
    WriteLog(QString("�����:  %1").arg(ScPackageGetFieldValue(sdh, "PP_Num2")));
    WriteLog(QString("������(�����������):  %1").arg(ScPackageGetFieldValue(sdh, "PP_Kem")));
    WriteLog(QString("������(����):  %1").arg(ScPackageGetFieldValue(sdh, "PP_Date")));
    WriteLog(QString("������(��� �������������):  %1").arg(ScPackageGetFieldValue(sdh, "PP_Podr")));
    WriteLog(QString("���� ����:  %1").arg(ScPackageGetFieldValue(sdh, "photo")));

    {
        QMutexLocker lock(&Data::mutex);

        Data::surnameEd = ScPackageGetFieldValue(sdh, "PP_SurName");
        Data::nameEd = ScPackageGetFieldValue(sdh, "PP_Name");
        Data::secondNameEd = ScPackageGetFieldValue(sdh, "PP_SecName");
        Data::sexEd = ScPackageGetFieldValue(sdh, "PP_Sex");
        Data::birthDateEd = ScPackageGetFieldValue(sdh, "PP_BirthDate");
        Data::birthPlaceEd = ScPackageGetFieldValue(sdh, "PP_BirthPlace");
        Data::serialEd = ScPackageGetFieldValue(sdh, "PP_Ser2");
        Data::numberEd = ScPackageGetFieldValue(sdh, "PP_Num2");
        Data::givenByUnit = ScPackageGetFieldValue(sdh, "PP_Kem");
        Data::issueDate = ScPackageGetFieldValue(sdh, "PP_Date");
        Data::givenByCode = ScPackageGetFieldValue(sdh, "PP_Podr");
        {
            ScPackageInformation packInfo;
            ScPackageGetInformation(scanyPackage, &packInfo);
            Data::photoFile = QString("%1\\%2").arg(packInfo.szName)
                                               .arg(ScPackageGetFieldValue(sdh, "photo"));
        }
    }

/*
    // 8. ����� ���������������� ���������� �������� ������ � ��������� ������
    if (ScPackageGetFirstDoc(psp))
            if (!ScPackageConvert(psp, "dss2txt.dll", CF_ROOT"Dataflow\\Export\\ScAPI"))
                    break;
*/

/*
    // 9. ����������� ������ � �����
    ScPackageMove(scanyPackage, CF_ROOT"Dataflow\\Archive");
    ScPackageClose(&scanyPackage);
*/

#endif

    emit DataProcessed();
}

ScanyThread::ScanyThread(QObject * parent) :
    QThread(parent),
    scanyInstance(NULL),
    scanyPackage(NULL),
    scanyDevice(NULL),
    scanyInitMutex(QMutex::Recursive),
    scanyRecognizeMutex(QMutex::Recursive)
{
}

bool ScanyThread::InitScanyApi()
{  
    if (!scanyInitMutex.tryLock()) {
        return 0;
    }
    QMutexLocker lock(&scanyInitMutex);
    scanyInitMutex.unlock();

#ifdef NO_SCANYAPY
    static bool first_time = false;
    if (first_time) return 1;
    first_time = true;

    SetProgressSteps(3);
    NextProgressStep("�������������");
    sleep(1);
    NextProgressStep("������������� ���� ����������� ��������");
    sleep(1);
    NextProgressStep("������������� ���������");
#else

    if (scanyInstance) {
        WriteLog("ScanyApi ��� ����������������");
        return 1;
    }

    // 1. ��������� �������� �-�� Scanify API
    WriteLog("������������� �������...");
    scanyInstance = ScInitialize();
    if (!scanyInstance) {
        WriteLog("�� ���� �������� ���������� ������ ScanyApi");
        WriteLog(QString("\n!������! %1\n").arg(ScGetErrorMessage(scanyInstance)));
        CloseScanyApi();
        return 0;
    }
    WriteLog("\tOK");

    // 1.2 ������ ScAPI
    WriteLog(QString("������� ������ ScAPI: %1").arg(ScGetScanifyAPIVersion()));

    // 2. ������������� ���� ����������� ��������
    ScSetupContext(scanyInstance, "scapi.ini");

#endif

    emit Initialized();
    return 1;
}

bool ScanyThread::CloseScanyApi()
{
#ifdef NO_SCANYAPY

#endif

    if (!scanyInstance) {
        WriteLog("ScapyApi �� ����������������");
    }

    if (scanyDevice) {
        ScScannerClose(scanyDevice);
        scanyDevice = NULL;
    }
    if (scanyPackage){
        ScPackageClose(&scanyPackage);
        scanyPackage = NULL;
        WriteLog("����� ������.");
    }
    if (scanyInstance){
        WriteLog("���������� ������ �������...");
        ScTerminate(scanyInstance);
        scanyInstance = NULL;
    }
}
