#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_playbackController(new PlaybackController)
{
    ui->setupUi(this);

    m_sequences = new TexturePlusDepthSequenceListModel();

    // assign models to list views
    ui->yuvListView->setModel(m_sequences);


    //
    // connect the objects with signals and slots
    //

    // connect playbackController
    connect(ui->nextFrameButton,SIGNAL(clicked(bool)),
            m_playbackController,SLOT(nextFrame()));
    connect(ui->previousFrameButton,SIGNAL(clicked(bool)),
            m_playbackController,SLOT(previousFrame()));

    connect(m_playbackController,SIGNAL(newSequenceFormat(int,int,int,int)),
            this,SLOT(updateGUIControls(int,int,int,int)));
    connect(m_playbackController,SIGNAL(newSequenceFormat(int,int,int,int)),
            ui->videoWidget,SLOT(updateFormat(int,int)));
    connect(m_playbackController,SIGNAL(newFrame(QByteArray)),
            ui->videoWidget,SLOT(updateFrame(QByteArray)));
    connect(ui->playbackLocationSlider,SIGNAL(valueChanged(int)),
            m_playbackController,SLOT(setFrame(int)));
    connect(ui->yuvListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)), // yuvListView to controller
            m_playbackController,SLOT(setSequence(QItemSelection)));
    connect(ui->playButton,SIGNAL(clicked(bool)),
            m_playbackController,SLOT(playOrPause()));
    connect(m_playbackController,SIGNAL(positionHasChanged(int)),
            this,SLOT(updatePosition(int)));

    // connect video widget
    connect(ui->videoWidget,SIGNAL(msSinceLastPaintChanged(int)),
            this,SLOT(updateFrameRate(int)));

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

void MainWindow::on_loadTextureAndDepthButton_clicked()
{
    QString fileNameTexture = QFileDialog::getOpenFileName(
                this,
                tr("Open sequence texture"),
                "",
                "Text files (*.yuv);;All files (*)");
    QString fileNameDepth  =     QString("/original/3D_FTV/PoznanBlocks/depth_8bit_400/Poznan_Blocks_d0_1920x1080_25.yuv");
//    if(fileNameDepth.isEmpty() || fileNameTexture.isEmpty()) return;

    // speed up debugging
    if(fileNameTexture.isEmpty())
    {
        fileNameTexture = QString("/original/3D_FTV/PoznanBlocks/1-ColorCorrected/Poznan_Blocks_t0_1920x1080_25.yuv");
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

void MainWindow::updateGUIControls(int frameWidth, int frameHeight, int numFrames, int frameRate)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    ui->frameCounterSpinBox->setMinimum(1);
    ui->frameCounterSpinBox->setMaximum(numFrames);
    ui->playbackLocationSlider->setMinimum(1);
    ui->playbackLocationSlider->setMaximum(numFrames);
}

void MainWindow::updatePosition(int frameIdx)
{
    ui->playbackLocationSlider->setValue(frameIdx);
    ui->frameCounterSpinBox->setValue(frameIdx);
}

void MainWindow::updateFrameRate(int msSinceLastPaint)
{
    double frameRate = 1000.0/msSinceLastPaint;
    ui->frameRateLabel->setText(QString::number(frameRate,'f',1));

}
