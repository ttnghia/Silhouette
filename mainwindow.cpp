//------------------------------------------------------------------------------------------
// mainwindow.cpp
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("Toon Shading & Silhouette Rendering");

    setupGUI();

    // Update continuously
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), renderer, SLOT(update()));
    timer->start(10);
}

//------------------------------------------------------------------------------------------
QSize MainWindow::sizeHint() const
{
    return QSize(1600, 1200);
}

//------------------------------------------------------------------------------------------
QSize MainWindow::minimumSizeHint() const
{
    return QSize(50, 50);
}

//------------------------------------------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        close();
        break;

    case Qt::Key_V:
        changeShadingMode(false);
        break;

    case Qt::Key_R:
        renderer->resetCameraPosition();
        break;

    case Qt::Key_M:
        nextMeshObjectTexture();
        break;

    default:
        renderer->keyPressEvent(e);
    }
}


//------------------------------------------------------------------------------------------
void MainWindow::setupGUI()
{
    renderer = new Renderer(this);

    ////////////////////////////////////////////////////////////////////////////////
    // shading modes
    QGridLayout* shadingLayout = new QGridLayout;


    QRadioButton* rdbToonShading = new QRadioButton("Toon Shading");
    rdb2ShadingMap[rdbToonShading] = ToonShading;
    shadingRDBList.append(rdbToonShading);
    shadingLayout->addWidget(rdbToonShading, 0, 0);

    QRadioButton* rdbPhongShading = new QRadioButton("Phong Shading");
    rdb2ShadingMap[rdbPhongShading] = PhongShading;
    shadingRDBList.append(rdbPhongShading);
    rdbPhongShading->setChecked(true);
    shadingLayout->addWidget(rdbPhongShading, 0, 1);
    
    QCheckBox* chkRenderSilhouette = new QCheckBox("Render Silhouette");
    shadingLayout->addWidget(chkRenderSilhouette, 1, 0, 1, 2);


    foreach (QRadioButton* rdbShading, rdb2ShadingMap.keys())
    {
        connect(rdbShading, SIGNAL(clicked(bool)), this, SLOT(changeShadingMode(bool)));
    }
    connect(chkRenderSilhouette, &QCheckBox::toggled, renderer, &Renderer::enableRenderSilhouette);

    QGroupBox* shadingGroup = new QGroupBox("Shading Modes");
    shadingGroup->setLayout(shadingLayout);


    ////////////////////////////////////////////////////////////////////////////////
    // mesh object textures
    QGridLayout* meshObjectTextureLayout = new QGridLayout;

    cbMeshObjectTexture = new QComboBox;
    meshObjectTextureLayout->addWidget(cbMeshObjectTexture, 0, 0, 1, 3);

    QToolButton* btnPreviousMeshObjectTexture = new QToolButton;
    btnPreviousMeshObjectTexture->setArrowType(Qt::LeftArrow);
    meshObjectTextureLayout->addWidget(btnPreviousMeshObjectTexture, 0, 3, 1, 1);

    QToolButton* btnNextMeshObjectTexture = new QToolButton;
    btnNextMeshObjectTexture->setArrowType(Qt::RightArrow);
    meshObjectTextureLayout->addWidget(btnNextMeshObjectTexture, 0, 4, 1, 1);

    QStringList* strListMeshObjectTexture = renderer->getStrListMeshObjectTexture();

    for(int i = 0; i < strListMeshObjectTexture->size(); ++i)
    {
        cbMeshObjectTexture->addItem(strListMeshObjectTexture->at(i));
    }

    cbMeshObjectTexture->setCurrentIndex(MetalTexture::CopperVerdigris);

    QGroupBox* meshObjectTextureGroup = new QGroupBox("Mesh Object Bump Map Texture");
    meshObjectTextureGroup->setLayout(meshObjectTextureLayout);

    connect(cbMeshObjectTexture, SIGNAL(currentIndexChanged(int)), renderer,
            SLOT(setMeshObjectTexture(int)));
    connect(btnPreviousMeshObjectTexture, SIGNAL(clicked()), this,
            SLOT(prevMeshObjectTexture()));
    connect(btnNextMeshObjectTexture, SIGNAL(clicked()), this, SLOT(nextMeshObjectTexture()));


    ////////////////////////////////////////////////////////////////////////////////
    // mesh object
    cbMeshObject = new QComboBox;
    QString str;
    str = QString("Teapot");
    cbMeshObject->addItem(str);

    str = QString("Bunny");
    cbMeshObject->addItem(str);
    QGridLayout* meshObjectLayout = new QGridLayout;
    meshObjectLayout->addWidget(cbMeshObject, 0, 0, 1, 3);
    QGroupBox* meshObjectGroup = new QGroupBox("Mesh Object");
    meshObjectGroup->setLayout(meshObjectLayout);

    QToolButton* btnPreviousMeshObject = new QToolButton;
    btnPreviousMeshObject->setArrowType(Qt::LeftArrow);
    meshObjectLayout->addWidget(btnPreviousMeshObject, 0, 3, 1, 1);

    QToolButton* btnNextMeshObject = new QToolButton;
    btnNextMeshObject->setArrowType(Qt::RightArrow);
    meshObjectLayout->addWidget(btnNextMeshObject, 0, 4, 1, 1);


    cbMeshObject->setCurrentIndex(MeshObject::BUNNY_OBJ);
    connect(cbMeshObject, SIGNAL(currentIndexChanged(int)), renderer,
            SLOT(setMeshObject(int)));
    connect(btnPreviousMeshObject, SIGNAL(clicked()), this,
            SLOT(prevMeshObject()));
    connect(btnNextMeshObject, SIGNAL(clicked()), this, SLOT(nextMeshObject()));



    ////////////////////////////////////////////////////////////////////////////////
    // light intensity
    QSlider* sldLightIntensity = new QSlider(Qt::Horizontal);
    sldLightIntensity->setMinimum(0);
    sldLightIntensity->setMaximum(100);
    sldLightIntensity->setValue(80);

    connect(sldLightIntensity, &QSlider::valueChanged, renderer,
            &Renderer::setLightIntensity);



    QSlider* sldAmbientLight = new QSlider(Qt::Horizontal);
    sldAmbientLight->setMinimum(0);
    sldAmbientLight->setMaximum(100);
    sldAmbientLight->setValue(30);

    connect(sldAmbientLight, &QSlider::valueChanged, renderer,
            &Renderer::setAmbientLightIntensity);


    QSlider* sldSpecularReflection = new QSlider(Qt::Horizontal);
    sldSpecularReflection->setMinimum(0);
    sldSpecularReflection->setMaximum(100);
    sldSpecularReflection->setValue(50);

    connect(sldSpecularReflection, &QSlider::valueChanged, renderer,
            &Renderer::setObjectsSpecularReflection);

    QGridLayout* lightIntensityLayout = new QGridLayout;
    lightIntensityLayout->addWidget(new QLabel("Directional:"), 0, 0);
    lightIntensityLayout->addWidget(sldLightIntensity, 0, 1);
    lightIntensityLayout->addWidget(new QLabel("Ambient:"), 1, 0);
    lightIntensityLayout->addWidget(sldAmbientLight, 1, 1);
    lightIntensityLayout->addWidget(new QLabel("Specular:"), 2, 0);
    lightIntensityLayout->addWidget(sldSpecularReflection, 2, 1);

    QGroupBox* lightIntensityGroup = new QGroupBox("All Objects' Light Intensity");
    lightIntensityGroup->setLayout(lightIntensityLayout);



    QPushButton* btnResetCamera = new QPushButton("Reset Camera");
    connect(btnResetCamera, &QPushButton::clicked, renderer,
            &Renderer::resetCameraPosition);


    ////////////////////////////////////////////////////////////////////////////////
    // Add slider group to parameter group
    QVBoxLayout* parameterLayout = new QVBoxLayout;
    parameterLayout->addWidget(shadingGroup);
    parameterLayout->addWidget(meshObjectTextureGroup);
    parameterLayout->addWidget(meshObjectGroup);
    parameterLayout->addWidget(lightIntensityGroup);

    parameterLayout->addWidget(btnResetCamera);


    parameterLayout->addStretch();

    QGroupBox* parameterGroup = new QGroupBox;
    parameterGroup->setFixedWidth(300);
    parameterGroup->setLayout(parameterLayout);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(renderer);
    hLayout->addWidget(parameterGroup);

    setLayout(hLayout);

}

//------------------------------------------------------------------------------------------
void MainWindow::changeShadingMode(bool _mouseClicked)
{
    QRadioButton* rdbShading;
    QRadioButton* rdbNextShading;

    for(int i = 0; i < shadingRDBList.size(); ++i)
    {
        rdbShading = shadingRDBList.at(i);

        if(rdbShading->isChecked())
        {
            if(!_mouseClicked)
            {
                rdbNextShading = shadingRDBList.at((i + 1) % shadingRDBList.size());
                rdbNextShading->setChecked(true);
                renderer->setShadingMode(rdb2ShadingMap.value(rdbNextShading));
            }
            else
            {
                renderer->setShadingMode(rdb2ShadingMap.value(rdbShading));
            }

            break;
        }
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::prevMeshObjectTexture()
{
    int index = cbMeshObjectTexture->currentIndex();

    if(index > 0)
    {
        cbMeshObjectTexture->setCurrentIndex(index - 1);
    }
    else
    {
        cbMeshObjectTexture->setCurrentIndex(cbMeshObjectTexture->count() - 1);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::nextMeshObjectTexture()
{
    int index = cbMeshObjectTexture->currentIndex();

    if(index < cbMeshObjectTexture->count() - 1)
    {
        cbMeshObjectTexture->setCurrentIndex(index + 1);
    }
    else
    {
        cbMeshObjectTexture->setCurrentIndex(0);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::prevMeshObject()
{
    int index = cbMeshObject->currentIndex();

    if(index > 0)
    {
        cbMeshObject->setCurrentIndex(index - 1);
    }
    else
    {
        cbMeshObject->setCurrentIndex(cbMeshObject->count() - 1);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::nextMeshObject()
{
    int index = cbMeshObject->currentIndex();

    if(index < cbMeshObject->count() - 1)
    {
        cbMeshObject->setCurrentIndex(index + 1);
    }
    else
    {
        cbMeshObject->setCurrentIndex(0);
    }
}
