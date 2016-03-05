//------------------------------------------------------------------------------------------
//
//
// Created on: 2/20/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------

#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <QOpenGLWidget>
#include <QList>
#include <QVector3D>
#include <QVector2D>
#include <math.h>

#include "cyTriMesh.h"

class OBJLoader
{
public:
    OBJLoader();
    ~OBJLoader();

    bool loadObjFile(const char *_fileName);

    int getNumVertices();
    int getVertexOffset();
    int getTexCoordOffset();
    float getScalingFactor();
    float getLowestYCoordinate();

    GLfloat* getVertices();
    GLfloat* getNormals();
    GLfloat* getTexureCoordinates();

private:
    cyTriMesh* objObject;
    cyPoint3f boxMin;
    cyPoint3f boxMax;

    void clearData();

    QVector<QVector3D> verticesList;
    QVector<QVector3D> normalsList;
    QVector<QVector2D> texCoordList;
};

#endif // OBJLOADER_H
