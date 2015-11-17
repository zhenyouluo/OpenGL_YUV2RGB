#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDoubleValidator>

#include <glm/glm.hpp>
#include "glm/gtx/string_cast.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cameraParameters (new CameraParameterListModel),
    m_playbackController(new PlaybackController)
{
    ui->setupUi(this);

    ui->zNearLineEdit->setValidator(new QDoubleValidator(0., 1000., 2));
    ui->zFarLineEdit->setValidator(new QDoubleValidator(0., 1000., 2));

    m_sequences = new TexturePlusDepthSequenceListModel();

    // assign models to list views
    ui->yuvListView->setModel(m_sequences);
    ui->camListView->setModel(m_cameraParameters);

    // connect the objects with signals and slots

    // connect playbackController
    connect(ui->nextFrameButton,SIGNAL(clicked(bool)),
            m_playbackController,SLOT(nextFrame()));
    connect(ui->previousFrameButton,SIGNAL(clicked(bool)),
            m_playbackController,SLOT(previousFrame()));

    connect(m_playbackController,SIGNAL(newSequenceFormat(int,int,int,int)),
            this,SLOT(updateGUIControls(int,int,int,int)));
    connect(m_playbackController,SIGNAL(newSequenceFormat(int,int,int,int)),
            ui->videoWidget,SLOT(updateFormat(int,int)));
    connect(m_playbackController,SIGNAL(newFrame(QByteArray,QByteArray)),
            ui->videoWidget,SLOT(updateFrame(QByteArray,QByteArray)));
    connect(ui->playbackLocationSlider,SIGNAL(valueChanged(int)),
            m_playbackController,SLOT(setFrame(int)));
    connect(ui->yuvListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)), // yuvListView to controller
            m_playbackController,SLOT(setSequence(QItemSelection)));

    // connect video widget
    connect(ui->camListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)), // camListView to video widget
            ui->videoWidget, SLOT(updateViewCamera(QItemSelection)));
    connect(this,SIGNAL(refCamChanged(CameraParameterSet)),
            ui->videoWidget,SLOT(updateReferenceCamera(CameraParameterSet)));
    connect(ui->zFarLineEdit,SIGNAL(editingFinished()),
            this,SLOT(updateZFar()));
    connect(ui->zNearLineEdit,SIGNAL(editingFinished()),
            this,SLOT(updateZNear()));

    //pure gui interconnections
    connect(ui->playbackLocationSlider,SIGNAL(valueChanged(int)),
            ui->frameCounterSpinBox,SLOT(setValue(int)));
    connect(ui->frameCounterSpinBox,SIGNAL(valueChanged(int)),
            ui->playbackLocationSlider,SLOT(setValue(int)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadCamParamsButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open camera parameters"),
                "",
                "Text files (*.txt);;All files (*)");
//    if(fileName.isEmpty()) return;

    // speed up debugging
    if(fileName.isEmpty())
        fileName = QString("/home/ient/Software/ViewSynthesis-OpenGL/build-Avaso-Desktop-Debug/cam_param_blocks.txt");

    m_cameraParameters->replaceListFromParameterFile(fileName);
}

void MainWindow::on_loadTextureAndDepthButton_clicked()
{
    QString fileNameTexture = QFileDialog::getOpenFileName(
                this,
                tr("Open sequence texture"),
                "",
                "Text files (*.yuv);;All files (*)");
    QString fileNameDepth = QFileDialog::getOpenFileName(
                this,
                tr("Open sequence depth"),
                "",
                "Text files (*.yuv);;All files (*)");

//    if(fileNameDepth.isEmpty() || fileNameTexture.isEmpty()) return;

    // speed up debugging
    if(fileNameDepth.isEmpty() || fileNameTexture.isEmpty())
    {
        fileNameTexture = QString("/original/3D_FTV/PoznanBlocks/1-ColorCorrected/Poznan_Blocks_t0_1920x1080_25.yuv");
        fileNameDepth =     QString("/original/3D_FTV/PoznanBlocks/depth_8bit_400/Poznan_Blocks_d0_1920x1080_25.yuv");
    }

    /// todo: only stores filenames, since playbackcontroller finds out format
    SequenceMetaDataItem sequence(fileNameTexture,fileNameDepth,0,0);
    m_sequences->insertItem(sequence);

}

void MainWindow::on_nextFrameButton_clicked()
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;
//    emit nextFrame();
    int frameIdx = ui->playbackLocationSlider->value() + 1;
    ui->playbackLocationSlider->setValue(frameIdx);
    ui->frameCounterSpinBox->setValue(frameIdx);
}

void MainWindow::on_previousFrameButton_clicked()
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;
    int frameIdx = ui->playbackLocationSlider->value() - 1;
    ui->playbackLocationSlider->setValue(frameIdx);
    ui->frameCounterSpinBox->setValue(frameIdx);
}

void MainWindow::on_setRefCamButton_clicked()
{
    CameraParameterSet refCam = ui->camListView->currentIndex().data(Qt::UserRole).value<CameraParameterSet>();

    emit refCamChanged(refCam);

//    m_K_view = camParams.getK();
//    m_R_view = camParams.getR();
//    m_t_view = camParams.getT();

//    //recompute opengl matrices
//    setupMatrices();

//    // draw from new perspective
//    update();
}

void MainWindow::updateGUIControls(int frameWidth, int frameHeight, int numFrames, int frameRate)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    ui->frameCounterSpinBox->setMinimum(1);
    ui->frameCounterSpinBox->setMaximum(numFrames);
    ui->playbackLocationSlider->setMinimum(1);
    ui->playbackLocationSlider->setMaximum(numFrames);
}

void MainWindow::updateZFar()
{
    float zFar = ui->zFarLineEdit->text().toDouble();
    ui->videoWidget->updateZFar(zFar);
}

void MainWindow::updateZNear()
{
    float zNear = ui->zNearLineEdit->text().toDouble();
    ui->videoWidget->updateZNear(zNear);
}
