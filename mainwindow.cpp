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
//    CreateRow(tr("����"), );


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
    mainLayout->addRow(comment, edit);
}
