#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QTextCodec>
void loadStyleSheet(QApplication& app)
{
    QFile file(":/config/style.qss");
    if(file.open(QFile::ReadOnly | QIODevice::Text)){
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
    }else{
        qDebug() << "load qss failed!";
    }
}
void initTextDefaultCodec() {
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());// 1
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());// 2
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());// 3
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    initTextDefaultCodec();
    loadStyleSheet(a);
    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
