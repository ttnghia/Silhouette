//------------------------------------------------------------------------------------------
//
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------
#include "renderer.h"

//------------------------------------------------------------------------------------------
Renderer::Renderer(QWidget* _parent):
    QOpenGLWidget(_parent),
    initializedScene(false),
    initializedTestScene(false),
    enabledRenderSilhouette(false),
    specialKeyPressed(Renderer::NO_KEY),
    mouseButtonPressed(Renderer::NO_BUTTON),
    translation(0.0f, 0.0f, 0.0f),
    translationLag(0.0f, 0.0f, 0.0f),
    rotation(0.0f, 0.0f, 0.0f),
    rotationLag(0.0f, 0.0f, 0.0f),
    zooming(0.0f),
    objLoader(NULL),
    cameraPosition(DEFAULT_CAMERA_POSITION),
    cameraFocus(DEFAULT_CAMERA_FOCUS),
    cameraUpDirection(0.0f, 1.0f, 0.0f),
    currentShadingMode(PhongShading),
    currentMeshObject(BUNNY_OBJ),
    currentMeshObjectTexture(CopperVerdigris),
    ambientLight(0.3)
{
    retinaScale = devicePixelRatio();
    setFocusPolicy(Qt::StrongFocus);

    ////////////////////////////////////////////////////////////////////////////////
    // mesh object texture
    strListMeshObjectTexture = new QStringList;
    strListMeshObjectTexture->append("AluminumTubing");
    strListMeshObjectTexture->append("BronzeAgeArtifact");
    strListMeshObjectTexture->append("CopperVerdigris");
    strListMeshObjectTexture->append("PyramidVerdigris");
    strListMeshObjectTexture->append("TexturedMetal");


    TRUE_OR_DIE(strListMeshObjectTexture->size() == NumMetalTextures,
                "Ohh, you forget to initialize some floor texture...");

}

//------------------------------------------------------------------------------------------
Renderer::~Renderer()
{
}

//------------------------------------------------------------------------------------------
void Renderer::checkOpenGLVersion()
{
    QString verStr = QString((const char*)glGetString(GL_VERSION));
    int major = verStr.left(verStr.indexOf(".")).toInt();
    int minor = verStr.mid(verStr.indexOf(".") + 1, 1).toInt();

    if(!(major >= 4 && minor >= 0))
    {
        QMessageBox::critical(this, "Error",
                              QString("Your OpenGL version is %1.%2, which does not satisfy this program requirement (OpenGL >= 4.0)")
                              .arg(major).arg(minor));
        exit(EXIT_FAILURE);
    }

//    qDebug() << major << minor;
//    qDebug() << verStr;
    //    TRUE_OR_DIE(major >= 4 && minor >= 1, "OpenGL version must >= 4.1");
}

//------------------------------------------------------------------------------------------
GLfloat triangle_vertices[] =
{
    0.0,  0.8, 0,
    -0.8, -0.8, 0,
    0.8, -0.8, 0
};

GLuint vbo = 0;
GLuint vao = 0;
QOpenGLShaderProgram* testProgram = new QOpenGLShaderProgram;

void Renderer::initTestScene()
{

    bool success;

    success = testProgram ->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                    ":/shaders/test.vs.glsl");
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = testProgram ->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                    ":/shaders/test.fs.glsl");
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = testProgram ->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    glUseProgram(testProgram ->programId());

    const char* attribute_name = "v_coord";
    GLint attribute_coord2d = glGetAttribLocation(testProgram->programId(), attribute_name);

    if (attribute_coord2d == -1)
    {
        qDebug() << "Could not bind attribute " << attribute_name;
        return;
    }

    glEnableVertexAttribArray(attribute_coord2d);


    glGenBuffers (1, &vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), triangle_vertices, GL_STATIC_DRAW);


    glGenVertexArrays (1, &vao);
//    glBindVertexArray (vao);
    glEnableVertexAttribArray(0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    initializedTestScene = true;
}

//------------------------------------------------------------------------------------------
void Renderer::initScene()
{
    TRUE_OR_DIE(initShaderPrograms(), "Cannot initialize shaders. Exit...");
    initTexture();
    initSceneMemory();
    initVertexArrayObjects();
    initSharedBlockUniform();
    initSceneMatrices();
}
//------------------------------------------------------------------------------------------
bool Renderer::initPhongShadingProgram()
{
    QOpenGLShaderProgram* program;
    GLint location;

    /////////////////////////////////////////////////////////////////
    glslPrograms[PhongShading] = new QOpenGLShaderProgram;
    program = glslPrograms[PhongShading];
    bool success;

    success = program->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                               vertexShaderSourceMap.value(PhongShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                               fragmentShaderSourceMap.value(PhongShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Geometry,
                                               ":/shaders/phong-shading.gs.glsl");
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    location = program->attributeLocation("v_coord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex coordinate.");
    attrVertex[PhongShading] = location;

    location = program->attributeLocation("v_normal");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex normal.");
    attrNormal[PhongShading] = location;

    location = program->attributeLocation("v_texCoord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex texCoord.");
    attrTexCoord[PhongShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Matrices");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMatrices[PhongShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Light");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniLight[PhongShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Material");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMaterial[PhongShading] = location;

    location = program->uniformLocation("cameraPosition");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform cameraPosition.");
    uniCameraPosition[PhongShading] = location;

    location = program->uniformLocation("ambientLight");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform ambientLight.");
    uniAmbientLight[PhongShading] = location;

    location = program->uniformLocation("objTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform objTex.");
    uniObjTexture[PhongShading] = location;

    location = program->uniformLocation("hasObjTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform hasObjTex.");
    uniHasObjTexture[PhongShading] = location;

    location = program->uniformLocation("normalTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform normalTex.");
    uniNormalTexture[PhongShading] = location;

    location = program->uniformLocation("hasNormalTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform hasNormalTex.");
    uniHasNormalTexture[PhongShading] = location;

    location = program->uniformLocation("needTangent");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform needTangent.");
    uniNeedTangent[PhongShading] = location;

    return true;
}

//------------------------------------------------------------------------------------------
bool Renderer::initToonShadingProgram()
{
    QOpenGLShaderProgram* program;
    GLint location;

    /////////////////////////////////////////////////////////////////
    glslPrograms[ToonShading] = new QOpenGLShaderProgram;
    program = glslPrograms[ToonShading];
    bool success;

    success = program->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                               vertexShaderSourceMap.value(ToonShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                               fragmentShaderSourceMap.value(ToonShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    location = program->attributeLocation("v_coord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex coordinate.");
    attrVertex[ToonShading] = location;

    location = program->attributeLocation("v_normal");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex normal.");
    attrNormal[ToonShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Matrices");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMatrices[ToonShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Light");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniLight[ToonShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Material");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMaterial[ToonShading] = location;

    location = program->uniformLocation("cameraPosition");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform cameraPosition.");
    uniCameraPosition[ToonShading] = location;

    location = program->uniformLocation("ambientLight");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform ambientLight.");
    uniAmbientLight[ToonShading] = location;

    return true;
}


//------------------------------------------------------------------------------------------
bool Renderer::initRenderSilhouetteProgram()
{
    GLint location;
    glslPrograms[ProgramRenderSilhouette] = new QOpenGLShaderProgram;
    QOpenGLShaderProgram* program = glslPrograms[ProgramRenderSilhouette];
    bool success;

    success = program->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                               vertexShaderSourceMap.value(ProgramRenderSilhouette));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                               fragmentShaderSourceMap.value(ProgramRenderSilhouette));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    location = program->attributeLocation("v_coord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex coordinate.");
    attrVertex[ProgramRenderSilhouette] = location;

    location = program->attributeLocation("v_normal");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex normal.");
    attrNormal[ProgramRenderSilhouette] = location;

    location = glGetUniformBlockIndex(program->programId(), "Matrices");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMatrices[ProgramRenderSilhouette] = location;

    return true;
}

//------------------------------------------------------------------------------------------
bool Renderer::initShaderPrograms()
{
    vertexShaderSourceMap.insert(ToonShading, ":/shaders/toon-shading.vs.glsl");
    vertexShaderSourceMap.insert(PhongShading, ":/shaders/phong-shading.vs.glsl");
    vertexShaderSourceMap.insert(ProgramRenderSilhouette,
                                 ":/shaders/silhouette.vs.glsl");

    fragmentShaderSourceMap.insert(ToonShading, ":/shaders/toon-shading.fs.glsl");
    fragmentShaderSourceMap.insert(PhongShading, ":/shaders/phong-shading.fs.glsl");
    fragmentShaderSourceMap.insert(ProgramRenderSilhouette,
                                   ":/shaders/silhouette.fs.glsl");


    return (initRenderSilhouetteProgram() &&
            initToonShadingProgram() &&
            initPhongShadingProgram());
}

//------------------------------------------------------------------------------------------
bool Renderer::validateShaderPrograms(ShadingProgram _shadingMode)
{
    GLint status;
    GLint logLen;
    GLchar log[1024];

    glValidateProgram(glslPrograms[_shadingMode]->programId());
    glGetProgramiv(glslPrograms[_shadingMode]->programId(), GL_VALIDATE_STATUS, &status);

    glGetProgramiv(glslPrograms[_shadingMode]->programId(), GL_INFO_LOG_LENGTH, &logLen);

    if(logLen > 0)
    {
        glGetProgramInfoLog(glslPrograms[_shadingMode]->programId(), logLen, &logLen, log);

        if(QString(log).trimmed().length() != 0)
        {
            qDebug() << "ShadingMode: " << _shadingMode << ", log: " << log;
        }
    }

    return (status == GL_TRUE);
}

//------------------------------------------------------------------------------------------
void Renderer::initSharedBlockUniform()
{
    /////////////////////////////////////////////////////////////////
    // setup the light and material
    light.direction = DEFAULT_LIGHT_DIRECTION;
    light.intensity = 0.8f;

    meshObjectMaterial.setDiffuse(QVector4D(0.82f, 0.45f, 1.0f, 1.0f));
    meshObjectMaterial.setSpecular(QVector4D(0.5f, 0.5f, 0.5f, 1.0f));
    meshObjectMaterial.shininess = 50.0f;


    /////////////////////////////////////////////////////////////////
    // setup binding points for block uniform
    for(int i = 0; i < NUM_BINDING_POINTS; ++i)
    {
        UBOBindingIndex[i] = i + 1;
    }

    /////////////////////////////////////////////////////////////////
    // setup data for block uniform
    glGenBuffers(1, &UBOMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 4 * SIZE_OF_MAT4, NULL,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBOLight);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
    glBufferData(GL_UNIFORM_BUFFER, light.getStructSize(),
                 &light, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBOMeshObjectMaterial);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMeshObjectMaterial);
    glBufferData(GL_UNIFORM_BUFFER, meshObjectMaterial.getStructSize(),
                 &meshObjectMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

//------------------------------------------------------------------------------------------
void Renderer::initTexture()
{

    ////////////////////////////////////////////////////////////////////////////////
    // mesh object texture
    for(int tex = 0; tex < NumMetalTextures; ++tex)
    {
        QString normalTexFile = QString(":/textures/metals/%1-NormalMap.png").
                        arg(strListMeshObjectTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(normalTexFile), "Cannot load texture from file.");
        QString colorTexFile = QString(":/textures/metals/%1-ColorMap.png").
                       arg(strListMeshObjectTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(colorTexFile), "Cannot load texture from file.");

        normalMapsMeshObject[tex] = new QOpenGLTexture(QImage(normalTexFile).mirrored());
        normalMapsMeshObject[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsMeshObject[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsMeshObject[tex]->setWrapMode(QOpenGLTexture::Repeat);

        colorMapsMeshObject[tex] = new QOpenGLTexture(QImage(colorTexFile).mirrored());
        colorMapsMeshObject[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsMeshObject[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsMeshObject[tex]->setWrapMode(QOpenGLTexture::Repeat);
    }
}

//------------------------------------------------------------------------------------------
void Renderer::initSceneMemory()
{
    initMeshObjectMemory();
}

//------------------------------------------------------------------------------------------
void Renderer::initMeshObjectMemory()
{
    if(!objLoader)
    {
        objLoader = new OBJLoader;
    }

    bool result = false;

    switch (currentMeshObject)
    {
    case TEAPOT_OBJ:
        result = objLoader->loadObjFile(":/obj/teapot.obj");
        break;

    case BUNNY_OBJ:
        result = objLoader->loadObjFile(":/obj/bunny.obj");
        break;

    default:
        break;
    }

    if(!result)
    {
        QMessageBox::critical(NULL, "Error", "Could not load OBJ file!");
        return;
    }

    if(vboMeshObject.isCreated())
    {
        vboMeshObject.destroy();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // init memory for sphere
    vboMeshObject.create();
    vboMeshObject.bind();
    vboMeshObject.allocate(2 * objLoader->getVertexOffset() +
                           objLoader->getTexCoordOffset());
    vboMeshObject.write(0, objLoader->getVertices(), objLoader->getVertexOffset());
    vboMeshObject.write(objLoader->getVertexOffset(), objLoader->getNormals(),
                        objLoader->getVertexOffset());
    vboMeshObject.write(2 * objLoader->getVertexOffset(), objLoader->getTexureCoordinates(),
                        objLoader->getTexCoordOffset());
    vboMeshObject.release();
}

//------------------------------------------------------------------------------------------
// record the buffer state by vertex array object
//------------------------------------------------------------------------------------------
void Renderer::initVertexArrayObjects()
{
    initMeshObjectVAO(PhongShading);
    initMeshObjectVAO(ToonShading);
    initMeshObjectVAO(ProgramRenderSilhouette);
}

//------------------------------------------------------------------------------------------
void Renderer::initMeshObjectVAO(ShadingProgram _shadingMode)
{
    if(vaoMeshObject[_shadingMode].isCreated())
    {
        vaoMeshObject[_shadingMode].destroy();
    }

    QOpenGLShaderProgram* program = glslPrograms[_shadingMode];

    vaoMeshObject[_shadingMode].create();
    vaoMeshObject[_shadingMode].bind();

    vboMeshObject.bind();
    program->enableAttributeArray(attrVertex[_shadingMode]);
    program->setAttributeBuffer(attrVertex[_shadingMode], GL_FLOAT, 0, 3);

    if(_shadingMode == PhongShading || _shadingMode == ToonShading
       || _shadingMode == ProgramRenderSilhouette)
    {
        program->enableAttributeArray(attrNormal[_shadingMode]);
        program->setAttributeBuffer(attrNormal[_shadingMode], GL_FLOAT,
                                    objLoader->getVertexOffset(), 3);
    }

    if(_shadingMode == PhongShading)
    {
        program->enableAttributeArray(attrTexCoord[_shadingMode]);
        program->setAttributeBuffer(attrTexCoord[_shadingMode], GL_FLOAT,
                                    2 * objLoader->getVertexOffset(), 2);
    }


    // release vao before vbo and ibo
    vaoMeshObject[_shadingMode].release();
    vboMeshObject.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initSceneMatrices()
{
    /////////////////////////////////////////////////////////////////
    // mesh object
    TRUE_OR_DIE(objLoader, "OBJLoader must be initialized first");
    meshObjectModelMatrix.setToIdentity();
    meshObjectModelMatrix.translate(DEFAULT_MESH_OBJECT_POSITION);
    meshObjectModelMatrix.scale(3.0f);

    if(currentMeshObject != TEAPOT_OBJ)
        meshObjectModelMatrix.translate(QVector3D(0, -2.0f * objLoader->getLowestYCoordinate(),
                                                  0));

    meshObjectModelMatrix.scale(2.0f / objLoader->getScalingFactor());

    if(currentMeshObject == TEAPOT_OBJ)
    {
        meshObjectModelMatrix.rotate(-90, 1, 0, 0);
    }

    meshObjectNormalMatrix = QMatrix4x4(meshObjectModelMatrix.normalMatrix());

}

//------------------------------------------------------------------------------------------
void Renderer::setAmbientLightIntensity(int _ambientLight)
{
    ambientLight = (float) _ambientLight / 100.0f;
}

//------------------------------------------------------------------------------------------
void Renderer::setLightIntensity(int _intensity)
{
    if(!isValid())
    {
        return;
    }

    makeCurrent();
    light.intensity = (GLfloat)_intensity / 100.0f;
    glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
    glBufferData(GL_UNIFORM_BUFFER, light.getStructSize(),
                 &light, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::setObjectsSpecularReflection(int _intensity)
{
    if(!isValid())
    {
        return;
    }

    makeCurrent();
    float specular = (float) _intensity / 100.0f;
    meshObjectMaterial.setSpecular(QVector4D(specular, specular, specular, 1.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMeshObjectMaterial);
    glBufferData(GL_UNIFORM_BUFFER, meshObjectMaterial.getStructSize(),
                 &meshObjectMaterial, GL_STREAM_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::setMeshObject(int _objectIndex)
{
    if(!isValid())
    {
        return;
    }

    if(_objectIndex < 0 || _objectIndex >= NUM_MESH_OBJECT)
    {
        return;
    }

    currentMeshObject = static_cast<MeshObject>(_objectIndex);
    makeCurrent();
    initMeshObjectMemory();
    initMeshObjectVAO(PhongShading);
    initMeshObjectVAO(ToonShading);
    initMeshObjectVAO(ProgramRenderSilhouette);

    /////////////////////////////////////////////////////////////////
    // mesh object
    TRUE_OR_DIE(objLoader, "OBJLoader must be initialized first");
    meshObjectModelMatrix.setToIdentity();
    meshObjectModelMatrix.translate(DEFAULT_MESH_OBJECT_POSITION);
    meshObjectModelMatrix.scale(3.0f);

    if(currentMeshObject != TEAPOT_OBJ)
        meshObjectModelMatrix.translate(QVector3D(0, -2.0f * objLoader->getLowestYCoordinate(),
                                                  0));

    meshObjectModelMatrix.scale(2.0f / objLoader->getScalingFactor());

    if(currentMeshObject == TEAPOT_OBJ)
    {
        meshObjectModelMatrix.rotate(-90, 1, 0, 0);
    }

    meshObjectNormalMatrix = QMatrix4x4(meshObjectModelMatrix.normalMatrix());

    doneCurrent();
}

//------------------------------------------------------------------------------------------
QStringList* Renderer::getStrListMeshObjectTexture()
{
    return strListMeshObjectTexture;
}

//------------------------------------------------------------------------------------------
void Renderer::setMeshObjectColor(float _r, float _g, float _b)
{
    if(!isValid())
    {
        return;
    }

    meshObjectMaterial.setDiffuse(QVector4D(_r, _g, _b, 1.0f));
    makeCurrent();
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMeshObjectMaterial);
    glBufferData(GL_UNIFORM_BUFFER, meshObjectMaterial.getStructSize(),
                 &meshObjectMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
}

//------------------------------------------------------------------------------------------
void Renderer::setMeshObjectTexture(int _texture)
{
    currentMeshObjectTexture = _texture;
}

//------------------------------------------------------------------------------------------
void Renderer::updateCamera()
{
    zoomCamera();

    /////////////////////////////////////////////////////////////////
    // flush camera data to uniform buffer
    viewMatrix.setToIdentity();
    viewMatrix.lookAt(cameraPosition, cameraFocus, cameraUpDirection);

    viewProjectionMatrix = projectionMatrix * viewMatrix;

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * SIZE_OF_MAT4, SIZE_OF_MAT4,
                    viewProjectionMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//------------------------------------------------------------------------------------------
QSize Renderer::sizeHint() const
{
    return QSize(1600, 1200);
}

//------------------------------------------------------------------------------------------
QSize Renderer::minimumSizeHint() const
{
    return QSize(50, 50);
}

//------------------------------------------------------------------------------------------
void Renderer::initializeGL()
{
    initializeOpenGLFunctions();
    checkOpenGLVersion();

    if(!initializedScene)
    {
        initScene();
        initializedScene = true;
    }
}

//------------------------------------------------------------------------------------------
void Renderer::resizeGL(int w, int h)
{
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(45, (float)w / (float)h, 0.1f, 1000.0f);
}

//------------------------------------------------------------------------------------------
void Renderer::paintGL()
{
    if(!initializedScene)
    {
        return;
    }

    translateCamera();
    rotateCamera();
    updateCamera();


    // render scene
    renderScene();

}

//-----------------------------------------------------------------------------------------
void Renderer::mousePressEvent(QMouseEvent* _event)
{
    lastMousePos = QVector2D(_event->localPos());

    if(_event->button() == Qt::RightButton)
    {
        mouseButtonPressed = RIGHT_BUTTON;
    }
    else
    {
        mouseButtonPressed = LEFT_BUTTON;

//        if(currentMouseTransTarget == TRANSFORM_LIGHT)
//        {
//            mouseButtonPressed = RIGHT_BUTTON;
//        }
    }
}

//-----------------------------------------------------------------------------------------
void Renderer::mouseMoveEvent(QMouseEvent* _event)
{
    QVector2D mouseMoved = QVector2D(_event->localPos()) - lastMousePos;

    switch(specialKeyPressed)
    {
    case Renderer::NO_KEY:
    {

        if(mouseButtonPressed == RIGHT_BUTTON)
        {
            translation.setX(translation.x() + mouseMoved.x() / 50.0f);
            translation.setY(translation.y() - mouseMoved.y() / 50.0f);
        }
        else
        {
            rotation.setX(rotation.x() - mouseMoved.x() / 5.0f);
            rotation.setY(rotation.y() - mouseMoved.y() / 5.0f);

            QPointF center = QPointF(0.5 * width(), 0.5 * height());
            QPointF escentricity = _event->localPos() - center;
            escentricity.setX(escentricity.x() / center.x());
            escentricity.setY(escentricity.y() / center.y());
            rotation.setZ(rotation.z() - (mouseMoved.x()*escentricity.y() - mouseMoved.y() *
                                          escentricity.x()) / 5.0f);
        }

    }
    break;

    case Renderer::SHIFT_KEY:
    {
        if(mouseButtonPressed == RIGHT_BUTTON)
        {
            QVector2D dir = mouseMoved.normalized();
            zooming += mouseMoved.length() * dir.x() / 500.0f;
        }
        else
        {
            rotation.setX(rotation.x() + mouseMoved.x() / 5.0f);
            rotation.setZ(rotation.z() + mouseMoved.y() / 5.0f);
        }
    }
    break;

    case Renderer::CTRL_KEY:
        break;

    }

    lastMousePos = QVector2D(_event->localPos());
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::mouseReleaseEvent(QMouseEvent* _event)
{
    mouseButtonPressed = NO_BUTTON;
}

//------------------------------------------------------------------------------------------
void Renderer::wheelEvent(QWheelEvent* _event)
{
    if(!_event->angleDelta().isNull())
    {
        zooming +=  (_event->angleDelta().x() + _event->angleDelta().y()) / 500.0f;
    }

    update();
}

//------------------------------------------------------------------------------------------
void Renderer::setShadingMode(ShadingProgram _shadingMode)
{
    currentShadingMode = _shadingMode;
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::resetCameraPosition()
{
    cameraPosition = DEFAULT_CAMERA_POSITION;
    cameraFocus = DEFAULT_CAMERA_FOCUS;
    cameraUpDirection = QVector3D(0.0f, 1.0f, 0.0f);

    update();
}

//------------------------------------------------------------------------------------------
void Renderer::enableDepthTest(bool _status)
{
    if(!isValid())
    {
        return;
    }

    makeCurrent();

    if(_status)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    doneCurrent();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::enableRenderSilhouette(bool _state)
{
    enabledRenderSilhouette = _state;
}

//------------------------------------------------------------------------------------------
void Renderer::keyPressEvent(QKeyEvent* _event)
{
    switch(_event->key())
    {
    case Qt::Key_Shift:
        specialKeyPressed = Renderer::SHIFT_KEY;
        break;

    case Qt::Key_Plus:
        zooming -= 0.1f;
        break;

    case Qt::Key_Minus:
        zooming += 0.1f;
        break;

    case Qt::Key_Up:
        translation += QVector3D(0.0f, 0.5f, 0.0f);
        break;

    case Qt::Key_Down:
        translation -= QVector3D(0.0f, 0.5f, 0.0f);
        break;

    case Qt::Key_Left:
        translation -= QVector3D(0.5f, 0.0f, 0.0f);
        break;

    case Qt::Key_Right:
        translation += QVector3D(0.5f, 0.0f, 0.0f);
        break;

    default:
        QOpenGLWidget::keyPressEvent(_event);
    }
}

//------------------------------------------------------------------------------------------
void Renderer::keyReleaseEvent(QKeyEvent* _event)
{
    specialKeyPressed = Renderer::NO_KEY;
}

//------------------------------------------------------------------------------------------
void Renderer::translateCamera()
{
    translation *= MOVING_INERTIA;

    if(translation.lengthSquared() < 1e-4)
    {
        return;
    }


    QVector3D eyeVector = cameraFocus - cameraPosition;
    float scale = sqrt(eyeVector.length()) * 0.01f;

    QVector3D u = cameraUpDirection;

    QVector3D v = QVector3D::crossProduct(eyeVector, u);
    u = QVector3D::crossProduct(v, eyeVector);
    u.normalize();
    v.normalize();

    cameraPosition -= scale * (translation.x() * v + translation.y() * u);
    cameraFocus -= scale * (translation.x() * v + translation.y() * u);
}

//------------------------------------------------------------------------------------------
void Renderer::rotateCamera()
{
    rotation *= MOVING_INERTIA;

    if(rotation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D nEyeVector = cameraPosition - cameraFocus ;
    QVector3D u = cameraUpDirection;
    QVector3D v = QVector3D::crossProduct(-nEyeVector, u);

    u = QVector3D::crossProduct(v, -nEyeVector);
    u.normalize();
    v.normalize();

    float scale = sqrt(nEyeVector.length()) * 0.02f;
    QQuaternion qRotation = QQuaternion::fromAxisAndAngle(v, rotation.y() * scale) *
                            QQuaternion::fromAxisAndAngle(u, rotation.x() * scale) *
                            QQuaternion::fromAxisAndAngle(nEyeVector, rotation.z() * scale);
    nEyeVector = qRotation.rotatedVector(nEyeVector);

    QQuaternion qCamRotation = QQuaternion::fromAxisAndAngle(v, rotation.y() * scale) *
                               QQuaternion::fromAxisAndAngle(nEyeVector, rotation.z() * scale);

    cameraPosition = cameraFocus + nEyeVector;
    cameraUpDirection = qCamRotation.rotatedVector(cameraUpDirection);
}

//------------------------------------------------------------------------------------------
void Renderer::zoomCamera()
{
    zooming *= MOVING_INERTIA;

    if(fabs(zooming) < 1e-4)
    {
        return;
    }

    QVector3D nEyeVector = cameraPosition - cameraFocus ;
    float len = nEyeVector.length();
    nEyeVector.normalize();

    len += sqrt(len) * zooming * 0.3f;

    if(len < 0.5f)
    {
        len = 0.5f;
    }

    cameraPosition = len * nEyeVector + cameraFocus;

}

//------------------------------------------------------------------------------------------
void Renderer::renderTestScene()
{
    if(!initializedTestScene)
    {
        initTestScene();
        return;
    }

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(testProgram->programId());
    glBindVertexArray (vao);
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays (GL_TRIANGLES, 0, 3);
    glUseProgram(0);
}

//------------------------------------------------------------------------------------------
void Renderer::renderScene()
{
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    renderObjects();
}

//------------------------------------------------------------------------------------------
void Renderer::renderObjects()
{
    QOpenGLShaderProgram* program;

    if(currentShadingMode == PhongShading)
    {
        program = glslPrograms[PhongShading];
        program->bind();
        program->setUniformValue(uniCameraPosition[PhongShading],
                                 cameraPosition);
        program->setUniformValue(uniObjTexture[PhongShading], 0);
        program->setUniformValue(uniNormalTexture[PhongShading], 1);
        program->setUniformValue(uniAmbientLight[PhongShading], ambientLight);

        glUniformBlockBinding(program->programId(), uniMatrices[PhongShading],
                              UBOBindingIndex[BINDING_MATRICES]);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MATRICES],
                         UBOMatrices);
        glUniformBlockBinding(program->programId(), uniLight[PhongShading],
                              UBOBindingIndex[BINDING_LIGHT]);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_LIGHT],
                         UBOLight);

        renderMeshObject(program);

        program->release();
    }
    else
    {
        program = glslPrograms[ToonShading];
        program->bind();
        program->setUniformValue(uniCameraPosition[ToonShading],
                                 cameraPosition);
        program->setUniformValue(uniAmbientLight[ToonShading], ambientLight);

        glUniformBlockBinding(program->programId(), uniMatrices[ToonShading],
                              UBOBindingIndex[BINDING_MATRICES]);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MATRICES],
                         UBOMatrices);
        glUniformBlockBinding(program->programId(), uniLight[ToonShading],
                              UBOBindingIndex[BINDING_LIGHT]);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_LIGHT],
                         UBOLight);

        renderMeshObject(program);
        program->release();

    }


    if(enabledRenderSilhouette)
    {
        program = glslPrograms[ProgramRenderSilhouette];
        program->bind();
        program->setUniformValue("silhouetteColor", SILHOUETTE_COLOR);
        program->setUniformValue("offset", SILHOUETTE_OFSET);

        glUniformBlockBinding(program->programId(), uniMatrices[ProgramRenderSilhouette],
                              UBOBindingIndex[BINDING_MATRICES]);
        glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MATRICES],
                         UBOMatrices);

        // render back face
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);

        renderSilhouetteMeshObject();

        glDisable(GL_CULL_FACE);
        program->release();
    }
}

//------------------------------------------------------------------------------------------
void Renderer::renderMeshObject(QOpenGLShaderProgram* _program)
{
    if(!vaoMeshObject[currentShadingMode].isCreated())
    {
        qDebug() << "vaoMeshObject is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    meshObjectModelMatrix.constData());
    glBufferSubData(GL_UNIFORM_BUFFER, SIZE_OF_MAT4, SIZE_OF_MAT4,
                    meshObjectNormalMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformBlockBinding(_program->programId(), uniMaterial[currentShadingMode],
                          UBOBindingIndex[BINDING_MESH_OBJECT_MATERIAL]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MESH_OBJECT_MATERIAL],
                     UBOMeshObjectMaterial);

    if(currentShadingMode == ToonShading)
    {
        vaoMeshObject[currentShadingMode].bind();
        glDrawArrays(GL_TRIANGLES, 0, objLoader->getNumVertices());
        vaoMeshObject[currentShadingMode].release();
    }
    else
    {
        /////////////////////////////////////////////////////////////////
        // set the uniform
        _program->setUniformValue(uniHasObjTexture[currentShadingMode], GL_TRUE);
        _program->setUniformValue(uniHasNormalTexture[currentShadingMode], GL_TRUE);
        _program->setUniformValue(uniNeedTangent[currentShadingMode], GL_TRUE);

        /////////////////////////////////////////////////////////////////
        // render the mesh object
        vaoMeshObject[currentShadingMode].bind();
        colorMapsMeshObject[currentMeshObjectTexture]->bind(0);
        normalMapsMeshObject[currentMeshObjectTexture]->bind(1);
        glDrawArrays(GL_TRIANGLES, 0, objLoader->getNumVertices());
        normalMapsMeshObject[currentMeshObjectTexture]->release();
        colorMapsMeshObject[currentMeshObjectTexture]->release();
        vaoMeshObject[currentShadingMode].release();
    }

}

//------------------------------------------------------------------------------------------
void Renderer::renderSilhouetteMeshObject()
{
    if(!vaoMeshObject[ProgramRenderSilhouette].isCreated())
    {
        qDebug() << "vaoMeshObject is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    meshObjectModelMatrix.constData());
    glBufferSubData(GL_UNIFORM_BUFFER, SIZE_OF_MAT4, SIZE_OF_MAT4,
                    meshObjectNormalMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    vaoMeshObject[ProgramRenderSilhouette].bind();
    glDrawArrays(GL_TRIANGLES, 0, objLoader->getNumVertices());
    vaoMeshObject[ProgramRenderSilhouette].release();

    glDisable(GL_CULL_FACE);
}


