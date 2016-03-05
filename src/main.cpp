//------------------------------------------------------------------------------------------
// main.cpp
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------

#include <QApplication>
#include <QSurfaceFormat>
#include <QtOpenGL/qgl.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 0);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow mainWindow;
    mainWindow.show();
    mainWindow.setGeometry( QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
                                              mainWindow.size(),
                                              qApp->desktop()->availableGeometry()));

    return a.exec();
}
