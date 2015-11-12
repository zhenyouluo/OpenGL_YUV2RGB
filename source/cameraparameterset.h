#ifndef CAMERAPARAMETERSET_H
#define CAMERAPARAMETERSET_H
#include <glm/glm.hpp>
#include <QList>
#include <QAbstractListModel>

class CameraParameterSet
{
public:
    CameraParameterSet(){}
    CameraParameterSet(QString cName, glm::mat3x3 matK, glm::mat3x3 matR, glm::vec3 vect)
        : camName(cName), K(matK), R(matR), t(vect) {}

    QString getCamName() const;
    glm::mat3x3 getK() const;
    glm::mat3x3 getR() const;
    glm::vec3 getT() const;

private:
    QString camName;
    glm::mat3x3 K; // camera calibration matrix
    glm::mat3x3 R; // camera rotation
    glm::vec3   t; // camera translation
};
Q_DECLARE_METATYPE(CameraParameterSet)


class CameraParameterListModel : public QAbstractListModel
{

public:
    CameraParameterListModel(){}

    // need to implement these interface functions to display it in QListView
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    // for adding elements
    bool replaceListFromParameterFile(const QString &fileName);

    /// todo: implement this too for well behaved model. what is it for?
    ////    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    QList<CameraParameterSet> m_cameraParameters;
};

#endif // CAMERAPARAMETERSET_H
