#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>

#include "cameraparameterset.h"
#include "textureplusdepthsequencelistmodel.h"
#include "playbackcontroller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_loadCamParamsButton_clicked();
    void on_loadTextureAndDepthButton_clicked();
    void on_nextFrameButton_clicked();
    void on_previousFrameButton_clicked();
    void updateGUIControls(int frameWidth, int frameHeight, int numFrames, int frameRate);

signals:
    void nextFrame();

private:
    Ui::MainWindow *ui;
    //QList<CameraParameterSet> m_cameraParameters;
    CameraParameterListModel  *m_cameraParameters;
    TexturePlusDepthSequenceListModel *m_sequences;
    PlaybackController *m_playbackController;

};

#endif // MAINWINDOW_H
