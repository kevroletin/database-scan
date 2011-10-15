#include <QtGui/QApplication>
#include <QTranslator>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (1) {
        // Do not use it in big projects
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("Windows-1251"));
        QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1251"));
    } else {
        QTranslator translator;
        translator.load("path_to_translations");
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
