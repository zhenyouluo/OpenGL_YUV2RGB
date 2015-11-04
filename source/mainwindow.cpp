#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <glm/glm.hpp>
#include "glm/gtx/string_cast.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // connect the objects with signals and slots
//    connect(ui->camListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            this,SLOT(cameraSelectionChanged(QItemSelection)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadCameraParameters()
{
    qWarning() << "test of slot";
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open camera parameters"),
                "",
                "Text files (*.txt);;All files (*)");


    m_cameraParameters = new CameraParameterListModel(fileName);

    // qt mvc magic
    ui->camListView->setModel(m_cameraParameters);

    /// why does it have to be here?
    connect(ui->camListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this,SLOT(cameraSelectionChanged(QItemSelection)));
    connect(ui->camListView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        ui->videoWidget, SLOT(cameraSelectionChanged(QItemSelection)));
}

void MainWindow::cameraSelectionChanged(QItemSelection newCamera)
{
    qDebug() << "tst" << newCamera.first();
///    ... how to acces it/ cast it/ whatever ...
///
    QVariant test = newCamera.indexes().first().data(Qt::UserRole);

//        ui->camListView->model()->data()

//        QModelIndexList indexes = selection.indexes();
//        foreach(const QModelIndex &index, indexes) {
//            QListWidgetItem *item = LoadedFilesListWidget->item(index.row());
//            // ...
//        }

}
