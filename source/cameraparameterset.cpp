#include "cameraparameterset.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

CameraParameterListModel::CameraParameterListModel(QString fileName, QObject *parent)  : QAbstractListModel(parent)
{
    cameraParameters = CameraParameterListModel::parseParameterFile(fileName);
}

QList<CameraParameterSet> CameraParameterListModel::parseParameterFile(QString fileName)
{

    QList<CameraParameterSet> cameraParameters;
    QFile paramFile(fileName);
    paramFile.open(QIODevice::ReadOnly);
    QTextStream paramInStream(&paramFile);
    glm::mat3x3 matK(0.f);
    glm::mat3x3 matR(0.f);
    glm::vec3   vect(0.f);

    // first read all data of one camera, parse it, then read next camera
    // ignore imcomplete camara parameter sets
    do
    {

        QStringList oneCam;
        QString line = paramInStream.readLine();
        // find beginning of next parameter set
        while(line.isEmpty())
        {
            line = paramInStream.readLine();
        }
        // read parameters
        while(!line.isEmpty())
        {
            oneCam.append(line);
            line = paramInStream.readLine();
        }

        // finished reading one parameter set
        if(oneCam.size() != 9) continue; // invalid set, ignore
        QString camName = oneCam.at(0);
        QString camLine1 = oneCam.at(1);
        QString camLine2 = oneCam.at(2);
        QString camLine3 = oneCam.at(3);
        QString camLine4 = oneCam.at(4);
        QString camLine5 = oneCam.at(5);
        QString camLine6 = oneCam.at(6);
        QString camLine7 = oneCam.at(7);
        QString camLine8 = oneCam.at(8);
        // conversion for easy parsing
        QTextStream Kline1(&camLine1, QIODevice::ReadOnly);
        QTextStream Kline2(&camLine2, QIODevice::ReadOnly);
        QTextStream Kline3(&camLine3, QIODevice::ReadOnly);
        QTextStream doNotCareline1(&camLine4, QIODevice::ReadOnly);
        QTextStream doNotCareline2(&camLine5, QIODevice::ReadOnly);
        QTextStream Rtline1(&camLine6, QIODevice::ReadOnly);
        QTextStream Rtline2(&camLine7, QIODevice::ReadOnly);
        QTextStream Rtline3(&camLine8, QIODevice::ReadOnly);
        // now the parsing
        Kline1 >> matK[0][0] >> matK[1][0] >> matK[2][0] ;
        Kline2 >> matK[0][1] >> matK[1][1] >> matK[2][1] ;
        Kline3 >> matK[0][2] >> matK[1][2] >> matK[2][2] ;
        Rtline1 >> matR[0][0] >> matR[1][0] >> matR[2][0] >> vect[0];
        Rtline2 >> matR[0][1] >> matR[1][1] >> matR[2][1] >> vect[1];
        Rtline3 >> matR[0][2] >> matR[1][2] >> matR[2][2] >> vect[2];

        float nc1, nc2;
        doNotCareline1 >> nc1;
        doNotCareline2 >> nc2;

        // check conformance with standard format
        if(     matK[0][1] != 0.f  || matK[0][2] != 0.f ||
                matK[1][2] != 0.f  || matK[2][2] != 1.f ||
                nc1 != 0        || nc1 != 0 )
        {
            qWarning() << camName << " not conforming to standard format.";
        }

        cameraParameters.append(CameraParameterSet(camName,matK,matR,vect));
    }
    while(!paramInStream.atEnd());

    qInfo() << "Read " << cameraParameters.count() << " camera parameter sets.";

    return cameraParameters;
}



/*!
    Returns the number of items in the camera parameter list as the number of rows
    in the model.
*/
int CameraParameterListModel::rowCount(const QModelIndex &parent) const
{
    return cameraParameters.count();
}


/*!
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    Any valid index that corresponds to a parameter set in the list causes that
    string to be returned.
*/
QVariant CameraParameterListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= cameraParameters.size())
        return QVariant();

    // pass complete object for internal usage
    if (role == Qt::UserRole)
    {
        QVariant cameraParameterSet;
        cameraParameterSet.setValue(cameraParameters.at(index.row()));
        return cameraParameterSet;
    }

    // pass only name for use by the gui
    if (role == Qt::DisplayRole)
        return cameraParameters.at(index.row()).getCamName();
    else
        return QVariant();
}

QString CameraParameterSet::getCamName() const
{
    return camName;
}
