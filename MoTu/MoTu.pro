#-------------------------------------------------
#
# Project created by QtCreator 2018-12-04T21:22:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MoTu
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    removal_vignetting.cpp \
    Dark_Channel.cpp \
    deblurring.cpp \
    pamshow.cpp

HEADERS += \
        mainwindow.h \
    Dark_Channel.h \
    pamshow.h

FORMS += \
        mainwindow.ui \
    pamshow.ui

RESOURCES += \
    res.qrc
    
RC_ICONS = "res/brush.ico"  # 只需添加这一行即可设置程序图标

DISTFILES +=