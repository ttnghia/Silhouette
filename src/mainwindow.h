//------------------------------------------------------------------------------------------
// mainwindow.h
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSPinbox>
#include <QtGui>
#include <QtWidgets>

#include "renderer.h"
#include "colorselector.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void keyPressEvent(QKeyEvent*);

    void setupGUI();

public slots:
    void changeShadingMode(bool _mouseClicked = true);
    void prevMeshObjectTexture();
    void nextMeshObjectTexture();
    void prevMeshObject();
    void nextMeshObject();

private:
    Renderer* renderer;

    QMap<QRadioButton*, ShadingProgram> rdb2ShadingMap;
    QList<QRadioButton*> shadingRDBList;

    QComboBox* cbMeshObject;

    QComboBox* cbMeshObjectTexture;

};

#endif // MAINWINDOW_H
