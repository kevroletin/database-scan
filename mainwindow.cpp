#include "mainwindow.h"
#include <QtGui>
#include <stdio.h>
#include "ScProxy.h"

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      mainLayout(new QVBoxLayout),
      formLayout(new QFormLayout),
      logTextEd(new QTextEdit)
{
    logTextEd->setReadOnly(1);

    QGroupBox* groupBox = new QGroupBox();
    groupBox->setLayout(formLayout);
    CreateRow(tr("�������").toAscii(), sernameEd);
    CreateRow(tr("���"), nameEd);
    CreateRow(tr("��������"), middlenameEd);
    CreateRow(tr("���"), genderEd);
    CreateRow(tr("�����, �����"), serialNumEd);
    CreateRow(tr("���� ��������"), birthDateEd);
    CreateRow(tr("����� ��������"), birthPlaceEd);
    CreateRow(tr("���� ��������� ��������"), issueData);
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
/*
id
�.�.�.
����� + �����
���� ��������
����� ��������
���
���� ��������� ��������
������ ��������������
��� �������������
����
*/
}

MainWindow::~MainWindow()
{

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

void MainWindow::LoadFromFile()
{
    // TODO;
}

void MainWindow::LoadFromScanner()
{
    // TODO:
}

#define CF_ROOT		"C:\\Program Files\\Cognitive\\ScanifyAPI\\"

void MainWindow::Recognize()
{
    bool result = 0;
    bool bFromFile = 1;

    ScInstance* pScI = NULL;
    ScPackage* psp = NULL;
    ScScanDevice* pScanDev = NULL;
    bool bRC = 0;
    do{
            // 1. ��������� �������� �-�� Scanify API
            WriteLog("������������� �������...");
            pScI = ScInitialize();
            if (!pScI) break;
            WriteLog("\tOK");

            // 1.2 ������ ScAPI
            WriteLog(QString("������� ������ ScAPI: %1").arg(ScGetScanifyAPIVersion()));

            // 2. ������������� ���� ����������� ��������
            ScSetupContext(pScI, "scapi.ini");

            // 3. �������� ������ ������
            psp = ScPackageCreate(pScI);
            if (!psp) break;
            WriteLog("����� ������");

            // 4. ����������� ���������� ������������
            if (!bFromFile){ // TWAIN-�������
                    // ����� ����� ������� ��� �������. ���� ������������ �����������
                    // ������������ ������ ������� ����������� ������:
                    ScScannerSelectTWAINSource(pScI, NULL);
                    const char* pszTWAINSource1 = "";
                    pScanDev = ScScannerOpen(pScI, SC_DEV_SCANNER, (void*)pszTWAINSource1);
            }

            // 5. ���� �������� � ����
            WriteLog("\n----------------- ���� �������� �� � ������������ ����");
            if (bFromFile) // ������� �� ����������
                    pScanDev = ScScannerOpen(pScI, SC_DEV_FILE, CF_ROOT"Dataflow\\Images\\passp_drive.jpg");
            if (!pScanDev) break;
            // 5.1 ������� ������� ��������� = ������� � �����
            // ��������! ���� �� �������� ����������� ��������������
            // ������� 2� ����������, ��������� ��������� ������������
            // �������������� ���������� � .ini-�����, �� ����
            // ������������ ��������� � �� ������������� ��� ����������
            // ������������ ���������� �� ������� �����������.
            // ��. [ScanPassportAndDriveLic]/Zone1, Zone2 � �.�
            ScScannerSetConfiguration(pScanDev, "ScanPassportAndDriveLic");
            // 5.2 ������������ ����������� � �����
            WriteLog("��� ������������...");
            ScImage* pImg = ScPackageScan(psp, pScanDev, NULL);
            WriteLog("\t���������");
            if (!pImg)
                    WriteLog("������������� 0 �����������!");

            // 5.3 ������������� ����������� � ������
            // (������������ ������ ����� ����������� �����������)
            WriteLog("��� �������������...");
            if (!ScPackageRecog(psp)){
                    WriteLog("������� ������������� ���������� ����������� �������!");
                    break;
            }
            WriteLog("\t���������");

            // (5.4) ������ ����������� (��������) ����� � ��������������� � ��������� ����������
            bool bDoExportImage = 1;
            if (bDoExportImage && pImg){
                    if (!ScImageResample(pImg, 120, 120)) break;
                    ScExportImageOptions exp = { SC_SAVE_JPG, 90, 0};
                    if (!ScImageExport(pImg, CF_ROOT"Dataflow\\Archive\\MyPassport.jpg", &exp)) break;
            }

            bool bProcBlanks = 0;
            if (bProcBlanks){
                    // 6. ���� �������
                    WriteLog("\n----------------- ���� �������");
                    if (bFromFile){ // ������� �� ����������
                            ScScannerClose(pScanDev);
                            pScanDev = ScScannerOpen(pScI, SC_DEV_FILE, CF_ROOT"Dataflow\\Images\\blank*.tif");
                    }
                    if (!pScanDev) break;
                    ScScannerSetConfiguration(pScanDev, "ScanBlanks");
                    ScPackageScan(psp, pScanDev, NULL);
                    ScPackageRecog(psp);
            }

            // 7. ������ �������� ������������ �����
//            bool bPrintWholePack = 1;
//            if (bPrintWholePack){
//                    // 7.A ������ ����� �����������
//                    if (!printPackage(psp)) break;
//            }else{
                    // 7.A ������ ����������� ����� ������� ��������� (��������)
                    ScDocument *sdh = ScPackageGetFirstDoc(psp);
                    if (sdh){
                            WriteLog(QString("�������:   %1").arg(ScPackageGetFieldValue(sdh, "PP_SurName")));
                            WriteLog(QString("���:       %1").arg(ScPackageGetFieldValue(sdh, "PP_Name")));
                            WriteLog(QString("��������:  %2").arg(ScPackageGetFieldValue(sdh, "PP_SecName")));
//            }
           }

//		// 8. ����� ���������������� ���������� �������� ������ � ��������� ������
//		if (ScPackageGetFirstDoc(psp))
//			if (!ScPackageConvert(psp, "dss2txt.dll", CF_ROOT"Dataflow\\Export\\ScAPI"))
//				break;

            // 9. ����������� ������ � �����
            ScPackageMove(psp, CF_ROOT"Dataflow\\Archive");

            bRC = 1;
    }while(0);

    if (!bRC)
            WriteLog(QString("\n!������! %1\n").arg(ScGetErrorMessage(pScI)));

    if (pScanDev) ScScannerClose(pScanDev); // �������� ���������� ������������
    if (psp){
            ScPackageClose(&psp);			// �������� ������
            WriteLog("����� ������.");
    }
    if (pScI){
            WriteLog("���������� ������ �������...");
            ScTerminate(pScI);			// �������� �������� �-�� Scanify API
    }

    return;
}
