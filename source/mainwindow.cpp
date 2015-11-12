#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <glm/glm.hpp>
#include "glm/gtx/string_cast.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_playbackController(new PlaybackController),
    m_cameraParameters (new CameraParameterListModel)
{
    ui->setupUi(this);

    m_sequences = new TexturePlusDepthSequenceListModel();

    // assign models to list views
    ui->yuvListView->setModel(m_sequences);
    ui->camListView->setModel(m_cameraParameters);

    // connect the objects with signals and slots

    // connect playbackController
    connect(ui->nextFrameButton,SIGNAL(clicked(bool)),m_playbackController,SLOT(nextFrame()));
    connect(ui->previousFrameButton,SIGNAL(clicked(bool)),m_playbackController,SLOT(previousFrame()));
    connect(ui->nextFrameButton,SIGNAL(clicked(bool)),m_playbackController,SLOT(nextFrame()));
    connect(ui->nextFrameButton,SIGNAL(clicked(bool)),m_playbackController,SLOT(nextFrame()));
    connect(m_playbackController,SIGNAL(newSequenceFormat(int,int,int,int)),this,SLOT(updateGUIControls(int,int,int,int)));
    connect(m_playbackController,SIGNAL(newSequenceFormat(int,int,int,int)),ui->videoWidget,SLOT(updateFormat(int,int)));
    connect(m_playbackController,SIGNAL(newFrame(QImage,QVector<float>)),ui->videoWidget,SLOT(updateFrame(QImage,QVector<float>)));
    connect(m_playbackController,SIGNAL(newFrame(QImage,QByteArray)),ui->videoWidget,SLOT(updateFrame(QImage,QByteArray)));
    connect(m_playbackController,SIGNAL(newFrame(QImage,QVector<uint8_t>)),ui->videoWidget,SLOT(updateFrame(QImage,QVector<uint8_t>)));


//    connect(m_playbackController,SIGNAL(newPlaybackSliderRange(int,int)),ui->playbackLocationSlider,SLOT(setRange(int,int)));
//    connect(m_playbackController,SIGNAL(newPlaybackSliderMaximum(int)),ui->playbackLocationSlider,SLOT(set));
//    connect(m_playbackController,SIGNAL(newPlaybackSliderRange(int,int)),ui->frameCounterSpinBox,SLOT(setRange(int,int)));

    // connect list views
    connect(ui->camListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)), // camListView to video widget
            ui->videoWidget, SLOT(cameraSelectionChanged(QItemSelection)));
    connect(ui->yuvListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)), // yuvListView to controller
            m_playbackController,SLOT(setSequence(QItemSelection)));

    //pure gui interconnections
    connect(ui->playbackLocationSlider,SIGNAL(valueChanged(int)),ui->frameCounterSpinBox,SLOT(setValue(int)));
    connect(ui->frameCounterSpinBox,SIGNAL(valueChanged(int)),ui->playbackLocationSlider,SLOT(setValue(int)));
//    connect(ui->camListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            this,SLOT(cameraSelectionChanged(QItemSelection)));

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
    if(fileName.isEmpty()) return;

//    // speed up debugging
//    QString fileName = QString("/home/ient/Software/ViewSynthesis-OpenGL/build-Avaso-Desktop-Debug/cam_param_blocks.txt");

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
    if(fileNameDepth.isEmpty() || fileNameTexture.isEmpty()) return;

//    // speed up debugging
//    QString fileNameTexture = QString("/original/3D_FTV/PoznanBlocks/1-ColorCorrected/Poznan_Blocks_t0_1920x1080_25.yuv");
//    QString fileNameDepth =     QString("/original/3D_FTV/PoznanBlocks/depth_8bit_400/Poznan_Blocks_d0_1920x1080_25.yuv");

    /// todo: only stores filenames, since playbackcontroller finds out format
    SequenceMetaDataItem sequence(fileNameTexture,fileNameDepth,0,0);
    m_sequences->insertItem(sequence);

}

void MainWindow::on_nextFrameButton_clicked()
{
    qWarning() << "test of slot0";
    emit nextFrame();

}

void MainWindow::on_previousFrameButton_clicked()
{
    qWarning() << "test of slot";

}

void MainWindow::updateGUIControls(int frameWidth, int frameHeight, int numFrames, int frameRate)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    ui->frameCounterSpinBox->setMinimum(1);
    ui->frameCounterSpinBox->setMaximum(numFrames);
    ui->playbackLocationSlider->setMinimum(1);
    ui->playbackLocationSlider->setMaximum(numFrames);
}
