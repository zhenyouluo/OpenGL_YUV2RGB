#ifndef TEXTUREPLUSDEPTHSEQUENCELISTMODEL_H
#define TEXTUREPLUSDEPTHSEQUENCELISTMODEL_H
#include <QList>
#include <QAbstractListModel>

/**
 * @brief The SequenceMetaData class
 * This class stores relevant information about a sequence, e.g. filenames of depth and texture,
 * frame width and height, ...
 */
class SequenceMetaDataItem
{
public:
    SequenceMetaDataItem(){}
    SequenceMetaDataItem(QString fNameTexture, QString fNameDepth, int frameWidth, int frameHeight)
        : m_fileNameTexture(fNameTexture), m_fileNameDepth(fNameDepth), m_frameWidth(frameWidth), m_frameHeight(frameHeight) {}


    QString fileNameTexture() const;
    QString fileNameDepth() const;
    int frameWidth() const;
    int frameHeight() const;

private:
    QString m_fileNameTexture;
    QString m_fileNameDepth;
    int m_frameWidth;
    int m_frameHeight;
};
Q_DECLARE_METATYPE(SequenceMetaDataItem)



class TexturePlusDepthSequenceListModel : public QAbstractListModel
{

public:
    TexturePlusDepthSequenceListModel(){}

    // need to implement these interface functions to display it in QListView
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool insertItem(SequenceMetaDataItem &item);

    /// todo: implement this too for well behaved model. what is it for?
    ////    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    QList<SequenceMetaDataItem> m_sequenceMetaData;
};

#endif // TEXTUREPLUSDEPTHSEQUENCELISTMODEL_H
