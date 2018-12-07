#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowOpacity(1);  // 这一句代码实现窗口的透明效果，函数里面的参数是透明度，1表示不透明
    //w.setWindowFlags(Qt::FramelessWindowHint);  // 隐藏窗口的标题栏和边框
    //w.setAttribute(Qt::WA_TranslucentBackground);
    w.show();

    return a.exec();
}
