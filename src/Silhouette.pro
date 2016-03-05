#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T09:10:42
#
#-------------------------------------------------

QT       += core gui
QT += opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Silhouette
TEMPLATE = app

#QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder


SOURCES += main.cpp\
        mainwindow.cpp \
    unitsphere.cpp \
    unitcube.cpp \
    unitplane.cpp \
    objloader.cpp \
    renderer.cpp \
    colorselector.cpp

HEADERS  += mainwindow.h \
    unitsphere.h \
    unitcube.h \
    unitplane.h \
    cyTriMesh.h \
    cyPoint.h \
    objloader.h \
    renderer.h \
    colorselector.h

RESOURCES += \
    shaders.qrc \
    textures.qrc \
    models.qrc
