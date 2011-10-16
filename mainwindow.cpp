#include "mainwindow.h"
#include <QtGui>
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      mainLayout(new QVBoxLayout),
      formLayout(new QFormLayout),
      logTextEd(new QTextEdit),
      scanyInstance(NULL),
      scanyPackage(NULL),
      scanyDevice(NULL)
{
    logTextEd->setReadOnly(1);

    QGroupBox* groupBox = new QGroupBox();
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
    photoLabel = new QLabel;
    formLayout->addRow(tr("����"), photoLabel);
    mainLayout->addWidget(groupBox);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    recognizeBtn = new QPushButton(tr("����������"));
    connect(recognizeBtn, SIGNAL(clicked()), this, SLOT(Recognize()));
    btnLayout->addWidget(recognizeBtn);
    mainLayout->addLayout(btnLayout);

    mainLayout->addWidget(logTextEd);

    setLayout(mainLayout);
    setWindowTitle(tr("�������� �������"));
}

MainWindow::~MainWindow()
{
    CloseScanyApi();
}

void MainWindow::CreateRow(QString comment, QLineEdit*& edit)
{
    edit = new QLineEdit;
    formLayout->addRow(comment, edit);
}

void MainWindow::WriteLog(QString msg)
{
    logTextEd->setPlainText(QString("%1\n%2").arg(msg).
                                              arg(logTextEd->toPlainText()));
}

bool MainWindow::InitScanyApi()
{
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
        return 0;
    }
    WriteLog("\tOK");

    // 1.2 ������ ScAPI
    WriteLog(QString("������� ������ ScAPI: %1").arg(ScGetScanifyAPIVersion()));

    // 2. ������������� ���� ����������� ��������
    ScSetupContext(scanyInstance, "scapi.ini");

    return 1;
}

bool MainWindow::CloseScanyApi()
{
    if (!scanyInstance) {
        WriteLog("ScapyApi �� ����������������");
    }
    if (scanyDevice) {
        ScScannerClose(scanyDevice);
    }
    if (scanyPackage){
        ScPackageClose(&scanyPackage);
        WriteLog("����� ������.");
    }
    if (scanyInstance){
        WriteLog("���������� ������ �������...");
        ScTerminate(scanyInstance);
    }
}

#define CF_ROOT		"C:\\Program Files\\Cognitive\\ScanifyAPI\\"

void MainWindow::Recognize()
{
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
    // ������������ ��������� � �� ������������� ��� ����������
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

    surnameEd->setText(ScPackageGetFieldValue(sdh, "PP_SurName"));
    nameEd->setText(ScPackageGetFieldValue(sdh, "PP_Name"));
    secondNameEd->setText(ScPackageGetFieldValue(sdh, "PP_SecName"));
    sexEd->setText(ScPackageGetFieldValue(sdh, "PP_Sex"));
    birthDateEd->setText(ScPackageGetFieldValue(sdh, "PP_BirthDate"));
    birthPlaceEd->setText(ScPackageGetFieldValue(sdh, "PP_BirthPlace"));
    serialEd->setText(ScPackageGetFieldValue(sdh, "PP_Ser2"));
    numberEd->setText(ScPackageGetFieldValue(sdh, "PP_Num2"));
    givenByUnit->setText(ScPackageGetFieldValue(sdh, "PP_Kem"));
    issueDate->setText(ScPackageGetFieldValue(sdh, "PP_Date"));
    givenByCode->setText(ScPackageGetFieldValue(sdh, "PP_Podr"));

    {
        ScPackageInformation packInfo;
        ScPackageGetInformation(scanyPackage, &packInfo);
        QString photoFile = QString("%1\\%2").arg(packInfo.szName)
                                            .arg(ScPackageGetFieldValue(sdh, "photo"));
        QPixmap pix;
        pix.load(photoFile);
        pix = pix.scaled(QSize(100, 100), Qt::KeepAspectRatio);
        QBuffer buff;
        pix.save(&buff);
        photoLabel->setPixmap(pix);
        WriteLog(QString("������ ���� � ����:  %1").arg(photoFile));
    }

/*
    // 8. ����� ���������������� ���������� �������� ������ � ��������� ������
    if (ScPackageGetFirstDoc(psp))
            if (!ScPackageConvert(psp, "dss2txt.dll", CF_ROOT"Dataflow\\Export\\ScAPI"))
                    break;
*/

    // 9. ����������� ������ � �����
    ScPackageMove(scanyPackage, CF_ROOT"Dataflow\\Archive");
    ScPackageClose(&scanyPackage);
    return;
}
