#include "playbackcontroller.h"

#include <QDebug>
#include <QImage>
#include <QElapsedTimer>
#include "textureplusdepthsequencelistmodel.h"
#include "yuvfile.h"



#if __STDC__ != 1
#    define restrict __restrict /* use implementation __ format */
#else
#    ifndef __STDC_VERSION__
#        define restrict __restrict /* use implementation __ format */
#    else
#        if __STDC_VERSION__ < 199901L
#            define restrict __restrict /* use implementation __ format */
#        else
#            /* all ok */
#        endif
#    endif
#endif

static unsigned char clp_buf[384+256+384];
static unsigned char *clip_buf = clp_buf+384;

PlaybackController::PlaybackController() : m_isPlaying(false)
{
    m_colorConversionMode = YUVC709ColorConversionType;

    // initialize clipping table
    memset(clp_buf, 0, 384);
    int i;
    for (i = 0; i < 256; i++) {
        clp_buf[384+i] = i;
    }
    memset(clp_buf+384+256, 255, 384);
}

void PlaybackController::nextFrame()
{
    // ensure current frame has valid index
    if( (m_currentFrame >= 1) && (m_currentFrame < m_numFrames))
    {
        m_currentFrame++;
        setFrame();
    }

}

void PlaybackController::previousFrame()
{
    // ensure current frame has valid index
    if( (m_currentFrame > 1) && (m_currentFrame <= m_numFrames))
    {
        m_currentFrame--;
        setFrame();
    }
}


void PlaybackController::setFrame(int frameIdx)
{        
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    if(m_currentFrame == frameIdx)
      return;

    m_currentFrame = frameIdx;
    setFrame();
}

void PlaybackController::setFrame()
{
  qDebug() << "Function Name: " << Q_FUNC_INFO;

  QElapsedTimer timer;
  timer.start();

  if (m_yuvTextureSource->pixelFormat() != YUVC_24RGBPixelFormat)
  {
      // read YUV444 frame from file - 16 bit LE words
      m_yuvTextureSource->getOneFrame(&m_tmpTextureBufferYUV444, m_currentFrame);


      // convert from YUV444 (planar) - 16 bit words to RGB888 (interleaved) color format (in place)
//        convertYUV2RGB(&m_tmpBufferYUV444, &m_conversionBuffer, YUVC_24RGBPixelFormat, m_yuvTextureSource->pixelFormat());
  }
  else
  {
      // read RGB24 frame from file
      m_yuvTextureSource->getOneFrame(&m_tmpTextureBufferYUV444, m_currentFrame);
  }

  // need to copy since buffer is modified again
//    QImage test= QImage((unsigned char*)m_conversionBuffer.data(),m_frameWidth,m_frameHeight,QImage::Format_RGB888).copy();
//    QImage test= QImage((unsigned char*)m_tmpBufferYUV444.data(),m_frameWidth,m_frameHeight,QImage::Format_RGB888).copy();

  // depthmap data, using luma as depth, ignoring chroma
  m_yuvDepthSource->getOneDepthFrame(&m_tmpBufferYUV444, m_currentFrame);

  qDebug() << "Reading and converting texture and depth took" << timer.elapsed() << "milliseconds";

  emit newFrame( m_tmpTextureBufferYUV444);
  emit positionHasChanged(m_currentFrame);

  int msSinceLastSentFrame = m_measureFPSSentTimer.restart();
  emit msSinceLastSentFrameChanged(msSinceLastSentFrame);
}


void PlaybackController::setSequence(QItemSelection sequence)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    if(sequence.isEmpty()) return;

    SequenceMetaDataItem sequenceMetaData = sequence.indexes().first().data(Qt::UserRole).value<SequenceMetaDataItem>();

    QString fileNameTexture = sequenceMetaData.fileNameTexture();
    QString fileNameDepth = sequenceMetaData.fileNameDepth();


    QFileInfo checkFileTexture(fileNameTexture);
    QFileInfo checkFileDepth(fileNameDepth);
    if( checkFileTexture.exists() && checkFileTexture.isFile()
     && checkFileDepth.exists()   && checkFileDepth.isFile())
    {
        // These are a files. Get the file extensions and open them.
        QString fileExt = checkFileTexture.suffix().toLower();
        if (fileExt == "yuv") {
            // Open YUV file
            m_yuvTextureSource = std::make_shared<YUVFile>(fileNameTexture);
        }
        fileExt = checkFileDepth.suffix().toLower();
        if (fileExt == "yuv") {
            // Open YUV file
            m_yuvDepthSource = std::make_shared<YUVFile>(fileNameDepth);
        }

        int widthTexture, widthDepth;
        int heightTexture, heightDepth;
        int numFramesTexture, numFramesDepth;
        double frameRateTexture, frameRateDepth;
        m_yuvTextureSource->getFormat(&widthTexture, &heightTexture, &numFramesTexture, &frameRateTexture);
        m_yuvDepthSource->getFormat(&widthDepth, &heightDepth, &numFramesDepth, &frameRateDepth);

        if(     widthDepth != widthTexture || heightDepth != heightTexture ||
                numFramesDepth != numFramesTexture || frameRateDepth != frameRateTexture)
        {
            qWarning() << "Texture and Depth have different format! (lenght, frame size or rate)";

            // using the length of the shorter sequence
            if(numFramesDepth<numFramesTexture)
                m_numFrames = numFramesDepth;
            else
                m_numFrames = numFramesTexture;
//            m_yuvDepthSource.reset();
//            m_yuvTextureSource.reset();
//            return;
        }

        m_frameWidth = widthTexture;
        m_frameHeight = heightTexture;

        m_frameRate = frameRateTexture;

        emit newSequenceFormat(m_frameWidth, m_frameHeight, m_numFrames, m_frameRate);

        // ensure current frame has valid index
        if( (m_currentFrame < 1) || (m_currentFrame > m_numFrames)) m_currentFrame = 1;

        setFrame(m_currentFrame);

    }


}

void PlaybackController::playOrPause()
{
      if(m_isPlaying)
    // stop playback
    {
        if(m_playBackTimer != NULL)
        {
            m_playBackTimer->stop();
            disconnect(m_playBackTimer.get(),SIGNAL(timeout()),this,SLOT(nextFrame()));
        }
        m_isPlaying = false;
    }
    else
    // start playback
    {
        m_playBackTimer = std::make_shared<QTimer>(this);
        connect(m_playBackTimer.get(),SIGNAL(timeout()),this,SLOT(nextFrame()));
        m_playBackTimer->start(1000/50);   // 1000 / fps
        m_isPlaying = true;
    }

}



void PlaybackController::convertYUV2RGB(QByteArray *sourceBuffer, QByteArray *targetBuffer, YUVCPixelFormatType targetPixelFormat, YUVCPixelFormatType srcPixelFormat)
{
    Q_ASSERT(targetPixelFormat == YUVC_24RGBPixelFormat);

    const int bps = YUVFile::bitsPerSample( srcPixelFormat );

    // make sure target buffer is big enough
    int srcBufferLength = sourceBuffer->size();
    Q_ASSERT( srcBufferLength%3 == 0 ); // YUV444 has 3 bytes per pixel
    int componentLength = 0;
    //buffer size changes depending on the bit depth
    if( targetBuffer->size() != srcBufferLength)
        targetBuffer->resize(srcBufferLength);
    if(bps == 8)
    {
        componentLength = srcBufferLength/3;
        if( targetBuffer->size() != srcBufferLength)
            targetBuffer->resize(srcBufferLength);
    }
    else if(bps==10)
    {
        componentLength = srcBufferLength/6;
        if( targetBuffer->size() != srcBufferLength/2)
            targetBuffer->resize(srcBufferLength/2);
    }


    const int yOffset = 16<<(bps-8);
    const int cZero = 128<<(bps-8);
    const int rgbMax = (1<<bps)-1;
    int yMult, rvMult, guMult, gvMult, buMult;

    unsigned char *dst = (unsigned char*)targetBuffer->data();

    if (bps == 8) {
        switch (m_colorConversionMode) {
        case YUVC601ColorConversionType:
            yMult =   76309;
            rvMult = 104597;
            guMult = -25675;
            gvMult = -53279;
            buMult = 132201;
            break;
        case YUVC2020ColorConversionType:
            yMult =   76309;
            rvMult = 110013;
            guMult = -12276;
            gvMult = -42626;
            buMult = 140363;
            break;
        case YUVC709ColorConversionType:
        default:
            yMult =   76309;
            rvMult = 117489;
            guMult = -13975;
            gvMult = -34925;
            buMult = 138438;
            break;
        }
        const unsigned char * restrict srcY = (unsigned char*)sourceBuffer->data();
        const unsigned char * restrict srcU = srcY + componentLength;
        const unsigned char * restrict srcV = srcU + componentLength;
        unsigned char * restrict dstMem = dst;

        int i;
#pragma omp parallel for default(none) private(i) shared(srcY,srcU,srcV,dstMem,yMult,rvMult,guMult,gvMult,buMult,clip_buf,componentLength)// num_threads(2)
        for (i = 0; i < componentLength; ++i) {
            const int Y_tmp = ((int)srcY[i] - yOffset) * yMult;
            const int U_tmp = (int)srcU[i] - cZero;
            const int V_tmp = (int)srcV[i] - cZero;

            const int R_tmp = (Y_tmp                  + V_tmp * rvMult ) >> 16;//32 to 16 bit conversion by left shifting
            const int G_tmp = (Y_tmp + U_tmp * guMult + V_tmp * gvMult ) >> 16;
            const int B_tmp = (Y_tmp + U_tmp * buMult                  ) >> 16;

            dstMem[3*i]   = clip_buf[R_tmp];
            dstMem[3*i+1] = clip_buf[G_tmp];
            dstMem[3*i+2] = clip_buf[B_tmp];
        }
    } else if (bps > 8 && bps <= 16) {
        switch (m_colorConversionMode) {
        case YUVC601ColorConversionType:
            yMult =   19535114;
            rvMult =  26776886;
            guMult =  -6572681;
            gvMult = -13639334;
            buMult =  33843539;
            break;
        case YUVC709ColorConversionType:
        default:

            yMult =   19535114;
            rvMult =  30077204;
            guMult =  -3577718;
            gvMult =  -8940735;
            buMult =  35440221;

        }
        if (bps < 16) {
            yMult  = (yMult  + (1<<(15-bps))) >> (16-bps);//32 bit values
            rvMult = (rvMult + (1<<(15-bps))) >> (16-bps);
            guMult = (guMult + (1<<(15-bps))) >> (16-bps);
            gvMult = (gvMult + (1<<(15-bps))) >> (16-bps);
            buMult = (buMult + (1<<(15-bps))) >> (16-bps);
        }
        const unsigned short *srcY = (unsigned short*)sourceBuffer->data();
        const unsigned short *srcU = srcY + componentLength;
        const unsigned short *srcV = srcU + componentLength;
        unsigned char *dstMem = dst;

        int i;
//#pragma omp parallel for default(none) private(i) shared(srcY,srcU,srcV,dstMem,yMult,rvMult,guMult,gvMult,buMult,componentLength) // num_threads(2)
        for (i = 0; i < componentLength; ++i) {
            qint64 Y_tmp = ((qint64)srcY[i] - yOffset)*yMult;
            qint64 U_tmp = (qint64)srcU[i]- cZero ;
            qint64 V_tmp = (qint64)srcV[i]- cZero ;
           // unsigned int temp = 0, temp1=0;

            qint64 R_tmp  = (Y_tmp                  + V_tmp * rvMult) >> (8+bps);
            dstMem[i*3]   = (R_tmp<0 ? 0 : (R_tmp>rgbMax ? rgbMax : R_tmp))>>(bps-8);
            qint64 G_tmp  = (Y_tmp + U_tmp * guMult + V_tmp * gvMult) >> (8+bps);
            dstMem[i*3+1] = (G_tmp<0 ? 0 : (G_tmp>rgbMax ? rgbMax : G_tmp))>>(bps-8);
            qint64 B_tmp  = (Y_tmp + U_tmp * buMult                 ) >> (8+bps);
            dstMem[i*3+2] = (B_tmp<0 ? 0 : (B_tmp>rgbMax ? rgbMax : B_tmp))>>(bps-8);
//the commented section uses RGB 30 format (10 bits per channel)

/*
            qint64 R_tmp  = ((Y_tmp                  + V_tmp * rvMult))>>(8+bps)   ;
            temp = (R_tmp<0 ? 0 : (R_tmp>rgbMax ? rgbMax : R_tmp));
            dstMem[i*4+3]   = ((temp>>4) & 0x3F);

            qint64 G_tmp  = ((Y_tmp + U_tmp * guMult + V_tmp * gvMult))>>(8+bps);
            temp1 = (G_tmp<0 ? 0 : (G_tmp>rgbMax ? rgbMax : G_tmp));
            dstMem[i*4+2] = ((temp<<4) & 0xF0 ) | ((temp1>>6) & 0x0F);

            qint64 B_tmp  = ((Y_tmp + U_tmp * buMult                 ))>>(8+bps) ;
            temp=0;
            temp = (B_tmp<0 ? 0 : (B_tmp>rgbMax ? rgbMax : B_tmp));
            dstMem[i*4+1] = ((temp1<<2)&0xFC) | ((temp>>8) & 0x03);
            dstMem[i*4] = temp & 0xFF;
*/
        }
    } else {
        printf("bitdepth %i not supported\n", bps);
    }
}
