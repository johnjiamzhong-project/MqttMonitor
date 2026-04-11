#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Fusion 风格是 Qt5 上暗黑主题 QSS 的最佳基底，
    // 能确保所有控件都正确响应自定义样式表。
    a.setStyle(QStyleFactory::create("Fusion"));

    QFile qss(":/src/ui/dark.qss");
    if (qss.open(QFile::ReadOnly))
        a.setStyleSheet(qss.readAll());

    MainWindow w;
    w.resize(1100, 680);
    w.show();

    return a.exec();
}
