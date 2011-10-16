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
    CreateRow(tr("Фамилия").toAscii(), surnameEd);
    CreateRow(tr("Имя"), nameEd);
    CreateRow(tr("Отчество"), secondNameEd);
    CreateRow(tr("Пол"), sexEd);
    CreateRow(tr("Серия"), serialEd);
    CreateRow(tr("Номер"), numberEd);
    CreateRow(tr("Дата рождения"), birthDateEd);
    CreateRow(tr("Место рождения"), birthPlaceEd);
    CreateRow(tr("Дата получения паспорта"), issueDate);
    CreateRow(tr("Выдано подразделением"), givenByUnit);
    CreateRow(tr("Код подразделения"), givenByCode);
    photoLabel = new QLabel;
    formLayout->addRow(tr("Фото"), photoLabel);
    mainLayout->addWidget(groupBox);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    recognizeBtn = new QPushButton(tr("Распознать"));
    connect(recognizeBtn, SIGNAL(clicked()), this, SLOT(Recognize()));
    btnLayout->addWidget(recognizeBtn);
    mainLayout->addLayout(btnLayout);

    mainLayout->addWidget(logTextEd);

    setLayout(mainLayout);
    setWindowTitle(tr("Добавить паспорт"));
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
        WriteLog("ScanyApi уже инициализирована");
        return 1;
    }

    // 1. Получение инстанса с-мы Scanify API
    WriteLog("Инициализация системы...");
    scanyInstance = ScInitialize();
    if (!scanyInstance) {
        WriteLog("Не могу получить глобальный объект ScanyApi");
        WriteLog(QString("\n!Ошибка! %1\n").arg(ScGetErrorMessage(scanyInstance)));
        return 0;
    }
    WriteLog("\tOK");

    // 1.2 Версия ScAPI
    WriteLog(QString("Текущая версия ScAPI: %1").arg(ScGetScanifyAPIVersion()));

    // 2. Устанавливает файл контекстных настроек
    ScSetupContext(scanyInstance, "scapi.ini");

    return 1;
}

bool MainWindow::CloseScanyApi()
{
    if (!scanyInstance) {
        WriteLog("ScapyApi не инициализирована");
    }
    if (scanyDevice) {
        ScScannerClose(scanyDevice);
    }
    if (scanyPackage){
        ScPackageClose(&scanyPackage);
        WriteLog("Пакет закрыт.");
    }
    if (scanyInstance){
        WriteLog("Завершение работы системы...");
        ScTerminate(scanyInstance);
    }
}

#define CF_ROOT		"C:\\Program Files\\Cognitive\\ScanifyAPI\\"

void MainWindow::Recognize()
{
    if (!InitScanyApi()) return;

    // 5. Ввод паспорта и прав
    WriteLog("\n----------------- Ввод Паспорта РФ и Водительских Прав");

    // 3. Создание нового пакета
    scanyPackage = ScPackageCreate(scanyInstance);
    if (!scanyPackage) {
        WriteLog("Не могу получить пакет ScanyApi");
        WriteLog(QString("\n!Ошибка! %1\n").arg(ScGetErrorMessage(scanyInstance)));
        return;
    }

    WriteLog("Пакет создан");

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

    // 5.1 Выбрать цепочку обработки = Паспорт и права
    // Внимание! Если на исходном изображении предполагается
    // наличие 2х документов, требуется проверить корректность
    // геометрической информации в .ini-файле, то есть
    // соответствия указанных в нём прямоугольных зон документов
    // расположению документов на входном изображении.
    // См. [ScanPassportAndDriveLic]/Zone1, Zone2 и т.п

    // 5.2 Сканирование изображений в пакет
    WriteLog("Идёт сканирование...");
    ScImage* pImg = ScPackageScan(scanyPackage, scanyDevice, NULL);
    WriteLog("\tзавершено");
    if (!pImg) {
            WriteLog("Отсканировано 0 изображений!");
    }

    ScScannerClose(scanyDevice);
    scanyDevice = NULL;

    // 5.3 Распознавание изображений в пакете
    // (распознаются только вновь добавленные изображения)
    WriteLog("Идёт распознавание...");
    if (!ScPackageRecog(scanyPackage)){
        WriteLog("Процесс распознавания завершился критической ошибкой!");
        return;
    }
    WriteLog("\tзавершено");

    // (5.4) Первое изображение (паспорта) ужать и сэкспортировать в отдельную директорию
    if (pImg){
        if (!ScImageResample(pImg, 120, 120)) {
            WriteLog("Не могу сжать изображение");
        } else {
            ScExportImageOptions exp = { SC_SAVE_JPG, 90, 0};
            if (!ScImageExport(pImg, "./Passport.jpg", &exp)) {
                WriteLog("Не могу экспортировать изображение");
            }
        }
    }

    // 7.A печать конктретных полей первого документа (паспорта)
    ScDocument *sdh = ScPackageGetFirstDoc(scanyPackage);
    if (!sdh) {
        WriteLog("Не могу получить документ из пачки");
        return;
    }

    WriteLog(QString("Фамилия:   %1").arg(ScPackageGetFieldValue(sdh, "PP_SurName")));
    WriteLog(QString("Имя:       %1").arg(ScPackageGetFieldValue(sdh, "PP_Name")));
    WriteLog(QString("Отчество:  %1").arg(ScPackageGetFieldValue(sdh, "PP_SecName")));
    WriteLog(QString("Пол:  %1").arg(ScPackageGetFieldValue(sdh, "PP_Sex")));
    WriteLog(QString("Дада рождения:  %1").arg(ScPackageGetFieldValue(sdh, "PP_BirthDate")));
    WriteLog(QString("Дада рождения:  %1").arg(ScPackageGetFieldValue(sdh, "PP_BirthPlace")));
    WriteLog(QString("Серия:  %1").arg(ScPackageGetFieldValue(sdh, "PP_Ser2")));
    WriteLog(QString("Номер:  %1").arg(ScPackageGetFieldValue(sdh, "PP_Num2")));
    WriteLog(QString("Выдано(организация):  %1").arg(ScPackageGetFieldValue(sdh, "PP_Kem")));
    WriteLog(QString("Выдано(дата):  %1").arg(ScPackageGetFieldValue(sdh, "PP_Date")));
    WriteLog(QString("Выдано(код подразделения):  %1").arg(ScPackageGetFieldValue(sdh, "PP_Podr")));
    WriteLog(QString("Файл фото:  %1").arg(ScPackageGetFieldValue(sdh, "photo")));

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
        WriteLog(QString("Полный путь к фото:  %1").arg(photoFile));
    }

/*
    // 8. Вызов пользовательской библиотеки экспорта пакета в текстовый формат
    if (ScPackageGetFirstDoc(psp))
            if (!ScPackageConvert(psp, "dss2txt.dll", CF_ROOT"Dataflow\\Export\\ScAPI"))
                    break;
*/

    // 9. Перемещение пакета в архив
    ScPackageMove(scanyPackage, CF_ROOT"Dataflow\\Archive");
    ScPackageClose(&scanyPackage);
    return;
}
