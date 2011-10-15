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
    CreateRow(tr("��� �������������"), gibenByCode);
    photoLabel = new QLabel;
    formLayout->addRow(tr("����"), photoLabel);
    mainLayout->addWidget(groupBox);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    loadBtn = new QPushButton(tr("�������"));
    connect(loadBtn, SIGNAL(clicked()), this, SLOT(LoadFromFile()));
    btnLayout->addWidget(loadBtn);
    scannBtn = new QPushButton(tr("�����������"));
    connect(scannBtn, SIGNAL(clicked()), this, SLOT(LoadFromScanner()));
    btnLayout->addWidget(scannBtn);
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

    // 3. �������� ������ ������
    scanyPackage = ScPackageCreate(scanyInstance);
    if (!scanyPackage) {
        WriteLog("�� ���� �������� ����� ScanyApi");
        WriteLog(QString("\n!������! %1\n").arg(ScGetErrorMessage(scanyInstance)));
        return 0;
    }

    WriteLog("����� ������");
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

void MainWindow::LoadFromFile()
{
    InitScanyApi();
    // 4. ����������� ���������� ������������
    scanyDevice = ScScannerOpen(scanyInstance, SC_DEV_FILE, CF_ROOT"Dataflow\\Images\\passp_drive.jpg");
    WriteLog(scanyDevice ? "���������� ������������ �� ����� �������" : "������ ��� �������� ��������� ������������");
}

void MainWindow::LoadFromScanner()
{
    if (!InitScanyApi()) return;
    // 4. ����������� ���������� ������������
    // ����� ����� ������� ��� �������. ���� ������������ �����������
    // ������������ ������ ������� ����������� ������:
    ScScannerSelectTWAINSource(scanyInstance, NULL);
    const char* pszTWAINSource1 = "";
    scanyDevice = ScScannerOpen(scanyInstance, SC_DEV_SCANNER, (void*)pszTWAINSource1);
    WriteLog(scanyDevice ? "���������� ������������ � ���������� �������" : "������ ��� �������� ��������� ������������");
}

void MainWindow::Recognize()
{
    if (!scanyDevice) {
        // TODO: �� ������� ������ �� ��������
        WriteLog("���������� ������������ �� �������. �������� ���� ����, ���� ������.");
        return;
    }
    // 5. ���� �������� � ����
    WriteLog("\n----------------- ���� �������� �� � ������������ ����");


    // 5.1 ������� ������� ��������� = ������� � �����
    // ��������! ���� �� �������� ����������� ��������������
    // ������� 2� ����������, ��������� ��������� ������������
    // �������������� ���������� � .ini-�����, �� ����
    // ������������ ��������� � �� ������������� ��� ����������
    // ������������ ���������� �� ������� �����������.
    // ��. [ScanPassportAndDriveLic]/Zone1, Zone2 � �.�
    ScScannerSetConfiguration(scanyDevice, "ScanPassportAndDriveLic");
    // 5.2 ������������ ����������� � �����
    WriteLog("��� ������������...");
    ScImage* pImg = ScPackageScan(scanyPackage, scanyDevice, NULL);
    WriteLog("\t���������");
    if (!pImg)
            WriteLog("������������� 0 �����������!");

    // 5.3 ������������� ����������� � ������
    // (������������ ������ ����� ����������� �����������)
    WriteLog("��� �������������...");
    if (!ScPackageRecog(scanyPackage)){
        WriteLog("������� ������������� ���������� ����������� �������!");
        return;
    }
    WriteLog("\t���������");

/*
    // (5.4) ������ ����������� (��������) ����� � ��������������� � ��������� ����������
    bool bDoExportImage = 1;
    if (bDoExportImage && pImg){
            if (!ScImageResample(pImg, 120, 120)) break;
            ScExportImageOptions exp = { SC_SAVE_JPG, 90, 0};
            if (!ScImageExport(pImg, CF_ROOT"Dataflow\\Archive\\MyPassport.jpg", &exp)) break;
    }
*/

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
    gibenByCode->setText(ScPackageGetFieldValue(sdh, "PP_Podr"));


/*
    // 8. ����� ���������������� ���������� �������� ������ � ��������� ������
    if (ScPackageGetFirstDoc(psp))
            if (!ScPackageConvert(psp, "dss2txt.dll", CF_ROOT"Dataflow\\Export\\ScAPI"))
                    break;
*/

    // 9. ����������� ������ � �����
    ScPackageMove(scanyPackage, CF_ROOT"Dataflow\\Archive");
    return;
}
