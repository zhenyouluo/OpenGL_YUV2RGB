#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include <QObject>
#include <QItemSelection>
#include <memory>
#include "yuvsource.h"

class PlaybackController : public QObject
{
    Q_OBJECT
public:
    PlaybackController();

    void loadDepthMap(const char *imagepath, int frameWidth, int frameHeight);
public slots:
    void nextFrame();
    void previousFrame();
    void setFrame(int frameIdx);
    void setSequence(QItemSelection sequence);

signals:
//    void newPlaybackSliderRange(int startFrame, int endFrame);
    void newSequenceFormat(int frameWidth, int frameHeight, int numFrames, int frameRate);
    void newFrame(const QImage &textureData, const QVector<float> &depthData);
    void newFrame(const QImage &textureData, const QVector<uint8_t> &depthData);
    void newFrame(const QImage &textureData, const QByteArray &depthData);

private:
    void convertYUV2RGB(QByteArray *sourceBuffer, QByteArray *targetBuffer, YUVCPixelFormatType targetPixelFormat, YUVCPixelFormatType srcPixelFormat);


//    QSharedPointer
    std::shared_ptr<YUVSource> m_yuvTextureSource;
    std::shared_ptr<YUVSource> m_yuvDepthSource;

    int m_frameWidth;
    int m_frameHeight;
    int m_numFrames;
    double m_frameRate;

    QVector<float> m_depth_data;
    QByteArray m_conversionBuffer;
    QByteArray m_tmpBufferYUV444;

    YUVCColorConversionType m_colorConversionMode;

};

#endif // PLAYBACKCONTROLLER_H
