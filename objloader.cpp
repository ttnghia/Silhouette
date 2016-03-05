//------------------------------------------------------------------------------------------
//
//
// Created on: 2/20/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------
#include <QtWidgets>
#include "objloader.h"
#include "cyPoint.h"

OBJLoader::OBJLoader():
    objObject(NULL)
{
}

//------------------------------------------------------------------------------------------
bool OBJLoader::loadObjFile(const char* _fileName)
{
    if(!objObject)
    {
        objObject = new cyTriMesh;
    }
    else
    {
        objObject->Clear();
    }

    if(!objObject->LoadFromFileObj(_fileName, false))
    {
        return false;
    }


    clearData();
    objObject->ComputeNormals();
    objObject->ComputeBoundingBox();
    boxMin = objObject->GetBoundMin();
    boxMax = objObject->GetBoundMax();


    for(int i = 0; i < objObject->NF(); ++i)
    {
        cyTriMesh::cyTriFace face = objObject->F(i);

        for(int i = 0; i < 3; ++i)
        {
            cyPoint3f vertex = objObject->V(face.v[i]);
            verticesList.append(QVector3D(vertex.x, vertex.y, vertex.z));
        }

        cyTriMesh::cyTriFace faceNormal = objObject->FN(i);

        for(int i = 0; i < 3; ++i)
        {
            cyPoint3f normal = objObject->VN(faceNormal.v[i]);
            normalsList.append(QVector3D(normal.x, normal.y, normal.z));
        }


        cyTriMesh::cyTriFace faceTex = objObject->FT(i);

        for(int i = 0; i < 3; ++i)
        {
            cyPoint3f tex = objObject->VT(faceTex.v[i]);
            texCoordList.append(QVector2D(tex.x, tex.y));
        }


    }

    return true;
}

//------------------------------------------------------------------------------------------
OBJLoader::~OBJLoader()
{
    clearData();
    delete objObject;
}

//------------------------------------------------------------------------------------------
int OBJLoader::getNumVertices()
{
    return verticesList.size();
}

//------------------------------------------------------------------------------------------
int OBJLoader::getVertexOffset()
{
    return (sizeof(GLfloat) * getNumVertices() * 3);
}

//------------------------------------------------------------------------------------------
float OBJLoader::getScalingFactor()
{
    float diff = fmax(fmax(boxMax.x - boxMin.x, boxMax.y - boxMin.y), boxMax.z - boxMin.z);
    return 0.5f * diff;

}

//------------------------------------------------------------------------------------------
float OBJLoader::getLowestYCoordinate()
{
    return (boxMin.y / getScalingFactor());
}

//------------------------------------------------------------------------------------------
int OBJLoader::getTexCoordOffset()
{
    return (sizeof(GLfloat) * getNumVertices() * 2);
}

//------------------------------------------------------------------------------------------
GLfloat* OBJLoader::getVertices()
{
    return (GLfloat*)verticesList.data();
}

//------------------------------------------------------------------------------------------
GLfloat* OBJLoader::getNormals()
{
    return (GLfloat*)normalsList.data();
}

//------------------------------------------------------------------------------------------
GLfloat* OBJLoader::getTexureCoordinates()
{
    return (GLfloat*)texCoordList.data();
}


//------------------------------------------------------------------------------------------
void OBJLoader::clearData()
{
    verticesList.clear();
    normalsList.clear();
    texCoordList.clear();
}

