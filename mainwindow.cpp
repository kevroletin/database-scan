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
    CreateRow(tr("Фамилия").toAscii(), sernameEd);
    CreateRow(tr("Имя"), nameEd);
    CreateRow(tr("Отчество"), middlenameEd);
    CreateRow(tr("Пол"), genderEd);
    CreateRow(tr("Серия, номер"), serialNumEd);
    CreateRow(tr("Дата рождения"), birthDateEd);
    CreateRow(tr("Место рождения"), birthPlaceEd);
    CreateRow(tr("Дата получения паспорта"), issueData);
    CreateRow(tr("Выдано подразделением"), givenByUnit);
    CreateRow(tr("Код подразделения"), gibenByCode);
    photoLabel = new QLabel;
    formLayout->addRow(tr("Фото"), photoLabel);
    mainLayout->addWidget(groupBox);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    loadBtn = new QPushButton(tr("Открыть"));
    connect(loadBtn, SIGNAL(clicked()), this, SLOT(LoadFromFile()));
    btnLayout->addWidget(loadBtn);
    scannBtn = new QPushButton(tr("Сканировать"));
    connect(scannBtn, SIGNAL(clicked()), this, SLOT(LoadFromScanner()));
    btnLayout->addWidget(scannBtn);
    recognizeBtn = new QPushButton(tr("Распознать"));
    connect(recognizeBtn, SIGNAL(clicked()), this, SLOT(Recognize()));
    btnLayout->addWidget(recognizeBtn);
    mainLayout->addLayout(btnLayout);

    mainLayout->addWidget(logTextEd);

    setLayout(mainLayout);
    setWindowTitle(tr("Добавить паспорт"));
/*
id
Ф.И.О.
серия + номер
дата рождения
место рождения
пол
дата получения паспорта
выдано подразделением
код подразделения
фото
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
            // 1. Получение инстанса с-мы Scanify API
            WriteLog("Инициализация системы...");
            pScI = ScInitialize();
            if (!pScI) break;
            WriteLog("\tOK");

            // 1.2 Версия ScAPI
            WriteLog(QString("Текущая версия ScAPI: %1").arg(ScGetScanifyAPIVersion()));

            // 2. Устанавливает файл контекстных настроек
            ScSetupContext(pScI, "scapi.ini");

            // 3. Создание нового пакета
            psp = ScPackageCreate(pScI);
            if (!psp) break;
            WriteLog("Пакет создан");

            // 4. Подключение устройства сканирования
            if (!bFromFile){ // TWAIN-драйвер
                    // Здесь можно указать имя сканера. Либо предоставить возможность
                    // пользователю самому выбрать посредством вызова:
                    ScScannerSelectTWAINSource(pScI, NULL);
                    const char* pszTWAINSource1 = "";
                    pScanDev = ScScannerOpen(pScI, SC_DEV_SCANNER, (void*)pszTWAINSource1);
            }

            // 5. Ввод паспорта и прав
            WriteLog("\n----------------- Ввод Паспорта РФ и Водительских Прав");
            if (bFromFile) // читалка из директории
                    pScanDev = ScScannerOpen(pScI, SC_DEV_FILE, CF_ROOT"Dataflow\\Images\\passp_drive.jpg");
            if (!pScanDev) break;
            // 5.1 Выбрать цепочку обработки = Паспорт и права
            // Внимание! Если на исходном изображении предполагается
            // наличие 2х документов, требуется проверить корректность
            // геометрической информации в .ini-файле, то есть
            // соответствия указанных в нём прямоугольных зон документов
            // расположению документов на входном изображении.
            // См. [ScanPassportAndDriveLic]/Zone1, Zone2 и т.п
            ScScannerSetConfiguration(pScanDev, "ScanPassportAndDriveLic");
            // 5.2 Сканирование изображений в пакет
            WriteLog("Идёт сканирование...");
            ScImage* pImg = ScPackageScan(psp, pScanDev, NULL);
            WriteLog("\tзавершено");
            if (!pImg)
                    WriteLog("Отсканировано 0 изображений!");

            // 5.3 Распознавание изображений в пакете
            // (распознаются только вновь добавленные изображения)
            WriteLog("Идёт распознавание...");
            if (!ScPackageRecog(psp)){
                    WriteLog("Процесс распознавания завершился критической ошибкой!");
                    break;
            }
            WriteLog("\tзавершено");

            // (5.4) Первое изображение (паспорта) ужать и сэкспортировать в отдельную директорию
            bool bDoExportImage = 1;
            if (bDoExportImage && pImg){
                    if (!ScImageResample(pImg, 120, 120)) break;
                    ScExportImageOptions exp = { SC_SAVE_JPG, 90, 0};
                    if (!ScImageExport(pImg, CF_ROOT"Dataflow\\Archive\\MyPassport.jpg", &exp)) break;
            }

            bool bProcBlanks = 0;
            if (bProcBlanks){
                    // 6. Ввод бланков
                    WriteLog("\n----------------- Ввод бланков");
                    if (bFromFile){ // читалка из директории
                            ScScannerClose(pScanDev);
                            pScanDev = ScScannerOpen(pScI, SC_DEV_FILE, CF_ROOT"Dataflow\\Images\\blank*.tif");
                    }
                    if (!pScanDev) break;
                    ScScannerSetConfiguration(pScanDev, "ScanBlanks");
                    ScPackageScan(psp, pScanDev, NULL);
                    ScPackageRecog(psp);
            }

            // 7. Чтение значений распознанных полей
//            bool bPrintWholePack = 1;
//            if (bPrintWholePack){
//                    // 7.A печать всего содержимого
//                    if (!printPackage(psp)) break;
//            }else{
                    // 7.A печать конктретных полей первого документа (паспорта)
                    ScDocument *sdh = ScPackageGetFirstDoc(psp);
                    if (sdh){
                            WriteLog(QString("Фамилия:   %1").arg(ScPackageGetFieldValue(sdh, "PP_SurName")));
                            WriteLog(QString("Имя:       %1").arg(ScPackageGetFieldValue(sdh, "PP_Name")));
                            WriteLog(QString("Отчество:  %2").arg(ScPackageGetFieldValue(sdh, "PP_SecName")));
//            }
           }

//		// 8. Вызов пользовательской библиотеки экспорта пакета в текстовый формат
//		if (ScPackageGetFirstDoc(psp))
//			if (!ScPackageConvert(psp, "dss2txt.dll", CF_ROOT"Dataflow\\Export\\ScAPI"))
//				break;

            // 9. Перемещение пакета в архив
            ScPackageMove(psp, CF_ROOT"Dataflow\\Archive");

            bRC = 1;
    }while(0);

    if (!bRC)
            WriteLog(QString("\n!Ошибка! %1\n").arg(ScGetErrorMessage(pScI)));

    if (pScanDev) ScScannerClose(pScanDev); // закрытие устройства сканирования
    if (psp){
            ScPackageClose(&psp);			// закрытие пакета
            WriteLog("Пакет закрыт.");
    }
    if (pScI){
            WriteLog("Завершение работы системы...");
            ScTerminate(pScI);			// закрытие инстанса с-мы Scanify API
    }

    return;
}
