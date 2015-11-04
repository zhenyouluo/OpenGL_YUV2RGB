#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cameraparameterset.h"
#include <QItemSelection>

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
    void loadCameraParameters();
    void cameraSelectionChanged(QItemSelection newCamera);

private:
    Ui::MainWindow *ui;
    //QList<CameraParameterSet> m_cameraParameters;
    CameraParameterListModel  *m_cameraParameters;

};

#endif // MAINWINDOW_H
