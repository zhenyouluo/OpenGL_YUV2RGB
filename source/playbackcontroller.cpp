#include "playbackcontroller.h"

#include <QDebug>
#include <QImage>
#include "textureplusdepthsequencelistmodel.h"
#include "yuvfile.h"
#include <Magick++.h>



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

PlaybackController::PlaybackController()
{
    setObjectName("playbackController");
//    QMetaObject::connectSlotsByName(parent());
//    QMetaObject::connectSlotsByName(this);
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
    qDebug() << "Function Name: " << Q_FUNC_INFO;
}

void PlaybackController::previousFrame()
{    
    qDebug() << "Function Name: " << Q_FUNC_INFO;
}

void PlaybackController::setFrame(int frameIdx)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;
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
//            m_yuvDepthSource.reset();
//            m_yuvTextureSource.reset();
//            return;
        }

        m_frameWidth = widthTexture;
        m_frameHeight = heightTexture;
        m_numFrames = numFramesTexture;
        m_frameRate = frameRateTexture;

        emit newSequenceFormat(m_frameWidth, m_frameHeight, m_numFrames, m_frameRate);





        if (m_yuvTextureSource->pixelFormat() != YUVC_24RGBPixelFormat)
        {
            // read YUV444 frame from file - 16 bit LE words
            m_yuvTextureSource->getOneFrame(&m_tmpBufferYUV444, 1);

            // convert from YUV444 (planar) - 16 bit words to RGB888 (interleaved) color format (in place)
            convertYUV2RGB(&m_tmpBufferYUV444, &m_conversionBuffer, YUVC_24RGBPixelFormat, m_yuvTextureSource->pixelFormat());
        }
        else
        {
            // read RGB24 frame from file
            m_yuvTextureSource->getOneFrame(&m_conversionBuffer, 1);
        }

        // need to copy since buffer is modified again
        QImage test= QImage((unsigned char*)m_conversionBuffer.data(),m_frameWidth,m_frameHeight,QImage::Format_RGB888).copy();


        // Load cube.png image
//        QImage test = QImage("Poznan_Blocks_t0_1920x1080_25_f1.bmp").mirrored();
//        QByteArray texture = QByteArray::fromRawData(reinterpret_cast<const char*>(test.constBits()),test.size().height()*test.size().width()*3);

        // depthmap data, using luma as depth, ignoring chroma
        m_yuvDepthSource->getOneDepthFrame(&m_tmpBufferYUV444, 1);

        // need to copy since buffer is modified again
        QImage testD =QImage((unsigned char*)m_conversionBuffer.data(),m_frameWidth,m_frameHeight,QImage::Format_RGB888).copy();


        loadDepthMap("Poznan_Blocks_d0_1920x1080_25_f1.bmp", m_frameWidth, m_frameHeight);
        QByteArray depth = QByteArray::fromRawData(reinterpret_cast<const char*>(m_depth_data.constData()),m_depth_data.size());

//        QVector<float> depthDataVec;
//        for(int i=0;i<m_depth_data.count();i++)
//        {
//    //        uint8_t *test = depthData.at(i);
//            int test2 = m_depth_data.at(i);
//    //        if(test2<0)
//    //        {
//                uint8_t test3 = test2;
//    //        }
//            if((float) test2 != m_depth_data.at(i))
//            {
//                float test4 = m_depth_data.at(i);
//                qDebug() << test2;
//            }
//            if((float) test2 != (float) m_tmpBufferYUV444.at(i))
//            {
//                float test4 = m_tmpBufferYUV444.at(i);
//                qDebug() << test2;
//            }
//            depthDataVec.append((float) test3);
//        }

        // as uint8_t
        QVector<uint8_t> depth_uint_8t;
        for(int i=0;i<m_depth_data.count();i++)
        {
            if(m_depth_data[i]<0) qDebug() << "should not happen";
            if(m_depth_data[i]>255) qDebug() << "should not happen";
            depth_uint_8t.append(m_depth_data[i]);

            if(depth_uint_8t[i]!=m_depth_data[i]) qDebug() << "should not happen";
        }

//        emit newFrame( test, m_depth_data);
//        emit newFrame( test, depth_uint_8t);

        emit newFrame( test, m_tmpBufferYUV444);

//        emit newPlaybackSliderMaximum(m_numFrames);
        // set our name (remove file extension)
//        int lastPoint = p_source->getName().lastIndexOf(".");
//        p_name = p_source->getName().left(lastPoint);
    }


}



void PlaybackController::loadDepthMap(const char * imagepath, int frameWidth, int frameHeight){

//    float * data_float;
    int imageSize;

    printf("Reading image %s\n", imagepath);

    Magick::InitializeMagick(NULL);

    // Construct the image object. Separating image construction from the
    // the read operation ensures that a failure to read the image file
    // doesn't render the image object useless.
    Magick::Image image;
    try {
        // Read a file into image object
        image.read( imagepath );

        int width = image.columns();
        int height = image.rows();

        //std::cout << image.magick() << std::endl;

        imageSize = width*height;
        // convert to float
//        data_float = new float [imageSize];
        m_depth_data.resize(imageSize);

        // get a "pixel cache" for the entire image
        Magick::PixelPacket *pixels = image.getPixels(0, 0, width, height);

        // now you can access single pixels like a vector
        int row = 0;
        int column = 0;
        for(size_t h = 0; h < height; h++)
        {
            for(size_t w = 0; w < width; w++)
            {
                int i = h*width + w;

                Magick::Color color = pixels[h * width + w];
                int test = (unsigned char) color.redQuantum();

//                if(data[i]<min) min = data[i];
//                if(data[i]>max) max = data[i];
    //            data_float[i] = w%50 + 50;
                m_depth_data[i] = test;
//                data_float[i] = test;
            }
        }



//        pixels[0] = Magick::Color(255, 0, 0);
        // Crop the image to specified size (width, height, xOffset, yOffset)
//        image.crop( Geometry(100,100, 100, 100) );

        // Write the image to a file
//        image.write( "x.gif" );
    }
    catch( Magick::Exception &error_ )
    {
//        std::cout << "Caught exception: " << error_.what() << std::endl;
//        return 1;
    }
//    return 0;



//    std::cout << "Read " << imageSize << " depth values" << std::endl;
//    std::cout << "With padding: " << imageSize << " depth values" << max << std::endl;
//    std::cout << "min: " << min << " max: " << max << std::endl;

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
