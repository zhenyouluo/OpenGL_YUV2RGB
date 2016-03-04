#include "textureplusdepthsequencelistmodel.h"


QString SequenceMetaDataItem::fileNameTexture() const
{
    return m_fileNameTexture;
}

/*QString SequenceMetaDataItem::fileNameDepth() const
{
    return m_fileNameDepth;
}*/

int SequenceMetaDataItem::frameWidth() const
{
    return m_frameWidth;
}

int SequenceMetaDataItem::frameHeight() const
{
    return m_frameHeight;
}

/*!
    Returns the number of items in the camera parameter list as the number of rows
    in the model.
*/
int TexturePlusDepthSequenceListModel::rowCount(const QModelIndex &parent) const
{
    return m_sequenceMetaData.count();
}


/*!
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    Otherwise return a string for the gui or the whole object for internal usage, depending on role.
*/
QVariant TexturePlusDepthSequenceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_sequenceMetaData.size())
        return QVariant();

    // pass complete object for internal usage
    if (role == Qt::UserRole)
    {
        QVariant sequenceMetaItem;
        sequenceMetaItem.setValue(m_sequenceMetaData.at(index.row()));
        return sequenceMetaItem;
    }

    // pass only name for use by the gui
    if (role == Qt::DisplayRole)
        return m_sequenceMetaData.at(index.row()).fileNameTexture();  ///todo: include depth in string? will be quite long ...
    else
        return QVariant();
}

bool TexturePlusDepthSequenceListModel::insertItem(SequenceMetaDataItem &item)
{
    int oldLast = m_sequenceMetaData.count();
    beginInsertRows(QModelIndex(),oldLast,oldLast + 1);
    m_sequenceMetaData.append(item);
    endInsertRows();

    return true;
}
