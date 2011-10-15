#include "mainwindow.h"
#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFormLayout* l = new QFormLayout;
    mainLayout = l;
    QGroupBox* groupBox = new QGroupBox();
    groupBox->setLayout(l);
    setCentralWidget(groupBox);

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
//    CreateRow(tr("Фото"), );


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
    mainLayout->addRow(comment, edit);
}
