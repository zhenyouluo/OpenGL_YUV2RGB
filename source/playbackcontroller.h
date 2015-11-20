#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include <QObject>
#include <QItemSelection>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>
#include "yuvsource.h"

class PlaybackController : public QObject
{
    Q_OBJECT
public:
    PlaybackController();

public slots:
    void nextFrame();
    void previousFrame();
    void setFrame(int frameIdx);
    void setSequence(QItemSelection sequence);
    void playOrPause();

signals:
    void newSequenceFormat(int frameWidth, int frameHeight, int numFrames, int frameRate);
    void newFrame(const QByteArray &textureData);
    void positionHasChanged(int frameIdx);
    void msSinceLastSentFrameChanged(int ms);

private:
    void setFrame();
    void convertYUV2RGB(QByteArray *sourceBuffer, QByteArray *targetBuffer, YUVCPixelFormatType targetPixelFormat, YUVCPixelFormatType srcPixelFormat);

    std::shared_ptr<YUVSource> m_yuvTextureSource;
    std::shared_ptr<YUVSource> m_yuvDepthSource;

    int m_currentFrame;

    int m_frameWidth;
    int m_frameHeight;
    int m_numFrames;
    double m_frameRate;

    QByteArray m_conversionBuffer;
    QByteArray m_tmpBufferYUV444;
    QByteArray m_tmpTextureBufferYUV444;

    YUVCColorConversionType m_colorConversionMode;

    std::shared_ptr<QTimer> m_playBackTimer;
    bool m_isPlaying;

    QElapsedTimer m_measureFPSSentTimer;

};

#endif // PLAYBACKCONTROLLER_H
