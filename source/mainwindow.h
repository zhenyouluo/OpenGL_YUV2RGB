#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>

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
    // automatically connected by naming scheme
    void on_loadTextureAndDepthButton_clicked();
//    void on_nextFrameButton_clicked();
//    void on_previousFrameButton_clicked();

    // manually connected
    void updateGUIControls(int frameWidth, int frameHeight, int PxlFormat, int numFrames, int frameRate);
    void updatePosition(int frameIdx);
    void updateFrameRate(int msSinceLastPaint);
    void updateFramesSentRate(int msSinceLastSentFrame);

signals:
    void nextFrame();

private:
    Ui::MainWindow *ui;
    TexturePlusDepthSequenceListModel *m_sequences;
    PlaybackController *m_playbackController;

};

#endif // MAINWINDOW_H
