//------------------------------------------------------------------------------------------
//
//
// Created on: 1/21/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------

#include <QOpenGLWidget>
#include <QList>
#include <QVector3D>
#include <QVector2D>
#include <math.h>

#ifndef UVSPHERE_H
#define UVSPHERE_H


class UnitSphere
{
public:
    UnitSphere();
    ~UnitSphere();

    void generateSphere(int _numStacks, int _numSlices);

    int getNumVertices();
    int getNumIndices();
    int getVertexOffset();
    int getTexCoordOffset();
    int getIndexOffset();

    GLfloat* getVertices();
    GLfloat* getNormals();
    GLfloat* getNegativeNormals();
    GLfloat* getTexureCoordinates();
    GLushort* getIndices();

    int numStacks;
    int numSlices;

private:
    void clearData();

    QList<QVector3D> verticesList;
    QList<QVector2D> texCoordList;
    QList<QVector3D> normalsList;
    QVector<GLushort> indicesList;
    GLfloat* vertices;
    GLfloat* texCoord;
    GLfloat* normals;

};

#endif // UVSPHERE_H
