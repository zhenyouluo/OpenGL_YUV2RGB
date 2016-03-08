/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"
// ImageMagick to read bmp

#include <QElapsedTimer>

#include <iostream>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),      
      m_frameWidth(1),
      m_frameHeight(1),
      m_vertice_indices_Vbo(QOpenGLBuffer::IndexBuffer),
      m_texture_Ydata(0),
      m_texture_Udata(0),
      m_texture_Vdata(0),
      m_horizontalSubSampling(1),
      m_verticalSubSampling(1),
      m_bytesperComponent(1),
      m_program(0)
{
    m_core = QCoreApplication::arguments().contains(QStringLiteral("--coreprofile"));
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    m_transparent = QCoreApplication::arguments().contains(QStringLiteral("--transparent"));
    if (m_transparent)
        setAttribute(Qt::WA_TranslucentBackground);

    QSizePolicy p(sizePolicy());
//    p.setHorizontalPolicy(QSizePolicy::Fixed);
//    p.setVerticalPolicy(QSizePolicy::Fixed);
    p.setHeightForWidth(true);
    setSizePolicy(p);


//  uncommenting this enables update on the vertical refresh of the monitor. however it somehow interferes with the file dialog
//    connect(this,SIGNAL(frameSwapped()),this,SLOT(update()));

}

GLWidget::~GLWidget()
{
    cleanup();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

int GLWidget::heightForWidth(int w) const
{
    if(m_frameWidth == 0) return w;

    double h = static_cast<double> (m_frameHeight) / m_frameWidth * w;
    return h;
}

void GLWidget::cleanup()
{
    makeCurrent();
    m_vertices_Vbo.destroy();
    m_textureLuma_coordinates_Vbo.destroy();
    m_textureChroma_coordinates_Vbo.destroy();
    m_vertice_indices_Vbo.destroy();
    delete m_program;
//    delete m_texture_data;
    m_program = 0;
    doneCurrent();
}

void GLWidget::updateFrame(const QByteArray &textureData)
{
    if(m_pixelFormat == YUVC_UnknownPixelFormat || textureData.isEmpty())
    {
        qDebug() << "Unvalid PixelFormat or empty texture Array";
        return;
    }

    QElapsedTimer timer;
    timer.start();

    const unsigned char *srcY = (unsigned char*)textureData.data();
    const unsigned char *srcU = srcY + m_componentLength + (m_swapUV * (m_componentLength / m_horizontalSubSampling) / m_verticalSubSampling);
    const unsigned char *srcV = srcY + m_componentLength + ((1 - m_swapUV) * (m_componentLength / m_horizontalSubSampling) / m_verticalSubSampling);


    //QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    //QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    // transmitting the YUV data as three different textures
    // Y on unit 0
    m_texture_Ydata = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_texture_Ydata->create();
    m_texture_Ydata->setSize(m_frameWidth,m_frameHeight);
    m_texture_Ydata->setFormat(QOpenGLTexture::R8_UNorm);
    m_texture_Ydata->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    m_texture_Ydata->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, srcY);
    // Set filtering modes for texture minification and  magnification
    m_texture_Ydata->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture_Ydata->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
//    m_texture_Ydata->setWrapMode(QOpenGLTexture::Repeat);
    m_texture_Ydata->setWrapMode(QOpenGLTexture::ClampToBorder);
//    m_texture_Ydata->bind(0);

    // U on unit 1
    m_texture_Udata = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_texture_Udata->create();
    m_texture_Udata->setSize(m_frameWidth/m_horizontalSubSampling,m_frameHeight/m_verticalSubSampling);
    m_texture_Udata->setFormat(QOpenGLTexture::R8_UNorm);
    m_texture_Udata->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    m_texture_Udata->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, srcU);
    // Set filtering modes for texture minification and  magnification
    m_texture_Udata->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture_Udata->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
//    m_texture_Udata->setWrapMode(QOpenGLTexture::Repeat);
    m_texture_Udata->setWrapMode(QOpenGLTexture::ClampToBorder);
//    m_texture_Udata->bind(1);

    // V on unit 2
    m_texture_Vdata = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_texture_Vdata->create();
    m_texture_Vdata->setSize(m_frameWidth/m_horizontalSubSampling, m_frameHeight/m_verticalSubSampling);
    m_texture_Vdata->setFormat(QOpenGLTexture::R8_UNorm);
    m_texture_Vdata->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    m_texture_Vdata->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, srcV);
    //Set filtering modes for texture minification and  magnification
    m_texture_Vdata->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture_Vdata->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
//    m_texture_Vdata->setWrapMode(QOpenGLTexture::Repeat);
    m_texture_Vdata->setWrapMode(QOpenGLTexture::ClampToBorder);
//    m_texture_Vdata->bind(2);

    qDebug() << "Moving data to graphics card took" << timer.elapsed() << "milliseconds";

    update();
}

void GLWidget::updateFormat(int frameWidth, int frameHeight, int PxlFormat)
{
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_textureFormat = QImage::Format_RGB32;

    m_pixelFormat = YUVCPixelFormatType(PxlFormat);
    m_horizontalSubSampling = YUVSource::horizontalSubSampling(m_pixelFormat);
    m_verticalSubSampling = YUVSource::verticalSubSampling(m_pixelFormat);
    m_bytesperComponent = YUVSource::bytePerComponent(m_pixelFormat);

    m_swapUV = 0;
    if(m_pixelFormat == YUVC_422YpCrCb8PlanarPixelFormat || m_pixelFormat == YUVC_444YpCrCb8PlanarPixelFormat)
        m_swapUV = 1;

    m_componentLength = m_frameWidth*m_frameHeight;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    // compute frame vertices and copy to buffer
    /*computeFrameVertices(m_frameWidth, m_frameHeight);
    m_vertices_Vbo.bind();
    m_vertices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertices_Vbo.allocate(&m_videoFrameTriangles_vertices[0], m_videoFrameTriangles_vertices.size()* sizeof(glm::vec3));

    // compute indices of vertices and copy to buffer
    computeFrameMesh(m_frameWidth, m_frameHeight);
    m_vertice_indices_Vbo.bind();
    m_vertice_indices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertice_indices_Vbo.allocate(&m_videoFrameTriangles_indices[0], m_videoFrameTriangles_indices.size() * sizeof(unsigned int));

    computeLumaTextureCoordinates(m_frameWidth, m_frameHeight);
    computeChromaTextureCoordinates(m_frameWidth/m_horizontalSubSampling, m_frameHeight/m_verticalSubSampling);

    m_textureChroma_coordinates_Vbo.bind();
    m_textureChroma_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_textureChroma_coordinates_Vbo.allocate(&m_videoFrameDataPoints_Chroma[0],
            m_videoFrameDataPoints_Chroma.size() * sizeof(float));

    std::cout << "nr of Luma coordinates produced:" << m_videoFrameDataPoints_Luma.size() / 2 << std::endl;
    std::cout << "nr of Chroma coordinates produced:" << m_videoFrameDataPoints_Chroma.size() / 2 << std::endl;

    m_textureLuma_coordinates_Vbo.bind();
    m_textureLuma_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_textureLuma_coordinates_Vbo.allocate(&m_videoFrameDataPoints_Luma[0], m_videoFrameDataPoints_Luma.size() * sizeof(float));

    setupMatrices();

    update();*/

    // just put video on two triangles forming a rectangle
    // compute frame vertices and copy to buffer

    m_videoFrameTriangles_vertices.clear();
    m_videoFrameTriangles_vertices.push_back(glm::vec3(-1,-1,0));
    m_videoFrameTriangles_vertices.push_back(glm::vec3(1,1,0));
    m_videoFrameTriangles_vertices.push_back(glm::vec3(-1,1,0));
    m_videoFrameTriangles_vertices.push_back(glm::vec3(1,-1,0));
    //First Triangle
    m_videoFrameTriangles_indices.push_back(0);
    m_videoFrameTriangles_indices.push_back(1);
    m_videoFrameTriangles_indices.push_back(2);
    // second triangle
    m_videoFrameTriangles_indices.push_back(3);
    //m_videoFrameTriangles_indices.push_back(1);
    //m_videoFrameTriangles_indices.push_back(3);
    m_videoFrameDataPoints_Luma.clear();
    m_videoFrameDataPoints_Luma.push_back(0.f);
    m_videoFrameDataPoints_Luma.push_back(0.f);
    m_videoFrameDataPoints_Luma.push_back(1.f);
    m_videoFrameDataPoints_Luma.push_back(1.f);
    m_videoFrameDataPoints_Luma.push_back(0.f);
    m_videoFrameDataPoints_Luma.push_back(1.f);
    m_videoFrameDataPoints_Luma.push_back(1.f);
    m_videoFrameDataPoints_Luma.push_back(0.f);
    m_vertices_Vbo.create();
    m_vertices_Vbo.bind();
    m_vertices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertices_Vbo.allocate(&m_videoFrameTriangles_vertices[0], m_videoFrameTriangles_vertices.size()* sizeof(glm::vec3));
    // compute indices of vertices and copy to buffer
    std::cout << "nr of triangles:" << m_videoFrameTriangles_indices.size() / 3 << std::endl;
    m_vertice_indices_Vbo.create();
    m_vertice_indices_Vbo.bind();
    m_vertice_indices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertice_indices_Vbo.allocate(&m_videoFrameTriangles_indices[0], m_videoFrameTriangles_indices.size() * sizeof(unsigned int));
    m_textureLuma_coordinates_Vbo.create();
    m_textureLuma_coordinates_Vbo.bind();
    m_textureLuma_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_textureLuma_coordinates_Vbo.allocate(&m_videoFrameDataPoints_Luma[0], m_videoFrameDataPoints_Luma.size() * sizeof(float));

    //recompute opengl matrices

    setupMatrices();
    update();

}


void GLWidget::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, m_transparent ? 0 : 1);
    // Dark blue background
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    setupMatrices();

    // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
    // implementations this is optional and support may not be present
    // at all. Nonetheless the below code works in all cases and makes
    // sure there is a VAO when one is needed.
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex,":shaders/vertexshader.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment,":shaders/fragmentYUV2RGBshader.glsl");
    m_program->link();
    m_program->bind();

    m_matMVP_Loc = m_program->uniformLocation("MVP");

    m_program->setUniformValue("textureSamplerRed", 0);
    m_program->setUniformValue("textureSamplerGreen", 1);
    m_program->setUniformValue("textureSamplerBlue", 2);

    m_vertices_Loc = m_program->attributeLocation("vertexPosition_modelspace");
    m_textureLuma_Loc = m_program->attributeLocation("vertexLuma");
    m_textureChroma_Loc = m_program->attributeLocation("vertexChroma");
    /*m_vertices_Loc = m_program->attributeLocation("vertexPosition_modelspace");
    m_texture_Loc = m_program->attributeLocation("vertexUV");*/

    // Create vertex buffer objects
    m_vertices_Vbo.create();
    m_vertice_indices_Vbo.create();
    // m_texture_coordinates_Vbo.create();
    m_textureLuma_coordinates_Vbo.create();
    m_textureChroma_coordinates_Vbo.create();

    // Store the vertex attribute bindings for the program.
    setupVertexAttribs();

    m_program->release();
}

void GLWidget::setupVertexAttribs()
{

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    // 1rst attribute buffer : vertices
    m_vertices_Vbo.bind();
    f->glEnableVertexAttribArray(m_vertices_Loc);
    f->glVertexAttribPointer(
        m_vertices_Loc,     // attribute.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    m_textureLuma_coordinates_Vbo.bind();
    f->glEnableVertexAttribArray(m_textureLuma_Loc);
    f->glVertexAttribPointer(
        m_textureLuma_Loc,                    // attribute.
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // 3nd attribute buffer : UVs Chroma
    m_textureChroma_coordinates_Vbo.bind();
    f->glEnableVertexAttribArray(m_textureChroma_Loc);
    f->glVertexAttribPointer(
        m_textureChroma_Loc,              // attribute.
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

}

void GLWidget::setupMatrices()
{    
    glm::mat3 K_view = glm::mat3(1.f);

    // Our ModelViewProjection : projection of model to different view and then to image
    m_MVP = getProjectionFromCamCalibration(K_view,1000,1);

    //    std::cout << "MVP: " << glm::to_string(m_MVP) << std::endl;
}

glm::mat4 GLWidget::getProjectionFromCamCalibration(glm::mat3 &calibrationMatrix, float clipFar, float clipNear)
{
    calibrationMatrix[0] = -1.f * calibrationMatrix[0];
    calibrationMatrix[1] = -1.f * calibrationMatrix[1];
    calibrationMatrix[2] = -1.f * calibrationMatrix[2];
    clipFar = -1.0 * clipFar;
    clipNear = -1.0 * clipNear;

    glm::mat4 perspective(calibrationMatrix);
    perspective[2][2] = clipNear + clipFar;
    perspective[3][2] = clipNear * clipFar;
    perspective[2][3] = -1.f;
    perspective[3][3] = 0.f;

    glm::mat4 toNDC = glm::ortho(0.f,(float) m_frameWidth,(float) m_frameHeight,0.f,clipNear,clipFar);

    glm::mat4 Projection2 = toNDC * perspective;


    std::cout << "perspective: " << glm::to_string(perspective) << std::endl;
    std::cout << "toNDC: " << glm::to_string(toNDC) << std::endl;
    std::cout << "Projection2: " << glm::to_string(Projection2) << std::endl;

    return Projection2;
}

void GLWidget::paintGL()
{
//    qDebug() << "Function Name: " << Q_FUNC_INFO;
    QElapsedTimer timer;
    timer.start();

    // nothing loaded yet
    if(m_texture_Ydata == NULL || m_texture_Udata == NULL || m_texture_Vdata == NULL) return;
//    if(m_texture_red_data == NULL ) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    m_texture_Ydata->bind(0);
    m_texture_Udata->bind(1);
    m_texture_Vdata->bind(2);

//    m_program->setUniformValue("textureSamplerRed", 0);
//    m_program->setUniformValue("textureSamplerGreen", 1);
//    m_program->setUniformValue("textureSamplerBlue", 2);



    glUniformMatrix4fv(m_matMVP_Loc, 1, GL_FALSE, &m_MVP[0][0]);


    m_vertice_indices_Vbo.bind();
    m_vertices_Vbo.bind();
    m_textureLuma_coordinates_Vbo.bind();
    //m_textureChroma_coordinates_Vbo.bind();

    glDrawElements(
        GL_TRIANGLE_STRIP,      // mode
        m_videoFrameTriangles_indices.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    m_program->release();

//    qDebug() << "Painting took" << timer.elapsed() << "milliseconds";
    int msSinceLastPaint = m_measureFPSTimer.restart();
    emit msSinceLastPaintChanged(msSinceLastPaint);
}

// function to generate the vertices corresponding to the pixels of a frame. Each vertex is centered at a pixel, borders are padded.
void GLWidget::computeFrameVertices(int frameWidth, int frameHeight)
{

    m_videoFrameTriangles_vertices.push_back(glm::vec3(-1,-1,0));

    m_videoFrameTriangles_vertices.push_back(glm::vec3(1,1,0));

    m_videoFrameTriangles_vertices.push_back(glm::vec3(-1,1,0));

    m_videoFrameTriangles_vertices.push_back(glm::vec3(1,-1,0));


    /*for( int h = 0; h < frameHeight; h++)
    {
        for(int w = 0; w < frameWidth; w++)
        {

            m_videoFrameTriangles_vertices.push_back(glm::vec3(w,h,0));
        }
    }*/
}

// function to create a mesh by connecting the vertices to triangles
void GLWidget::computeFrameMesh(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_indices.clear();

    m_videoFrameTriangles_indices.push_back(2);
    m_videoFrameTriangles_indices.push_back(0);
    m_videoFrameTriangles_indices.push_back(1);
    // second triangle
    m_videoFrameTriangles_indices.push_back(0);
    m_videoFrameTriangles_indices.push_back(3);
    m_videoFrameTriangles_indices.push_back(1);

    /*for( int h = 0; h < frameHeight; h++)
    {
        for(int w = 0; w < frameWidth; w++)
        {
            //tblr: top bottom left right
            int index_pixel_tl = h*(frameWidth) + w;
            int index_pixel_tr = index_pixel_tl + 1;
            int index_pixel_bl = (h+1)*(frameWidth) + w;
            int index_pixel_br = index_pixel_bl + 1;

            // each pixel generates two triangles, specify them so orientation is counter clockwise
            // first triangle
            m_videoFrameTriangles_indices.push_back(index_pixel_tl);
            m_videoFrameTriangles_indices.push_back(index_pixel_bl);
            m_videoFrameTriangles_indices.push_back(index_pixel_tr);
            // second triangle
            m_videoFrameTriangles_indices.push_back(index_pixel_bl);
            m_videoFrameTriangles_indices.push_back(index_pixel_br);
            m_videoFrameTriangles_indices.push_back(index_pixel_tr);
        }
    }*/
}

// Function to fill Vector which will hold the texture coordinates which maps the texture (the video data) to the frame's vertices.
void GLWidget::computeLumaTextureCoordinates(int chromaWidth, int chromaHeight)
{
    m_videoFrameDataPoints_Luma.clear();

    for( int h = 0; h < chromaHeight; h++)
    {
        for(int w = 0; w < chromaWidth; w++)
        {

            float x,y;
            x = (w + 0.5)/chromaWidth;  //center of pixel
            y = (h + 0.5)/chromaHeight;  //center of pixel

            // set u for the pixel
            m_videoFrameDataPoints_Luma.push_back(x);
            // set v for the pixel
            m_videoFrameDataPoints_Luma.push_back(y);
            // set u for the pixel
        }
    }
}

void GLWidget::computeChromaTextureCoordinates(int chromaframeWidth, int chromaframeHeight)
{
    m_videoFrameDataPoints_Chroma.clear();

    for( int h = 0; h < chromaframeHeight; h++)
    {
        for(int i = 0; (i < m_verticalSubSampling); i++)
        {
            for(int w = 0; w < chromaframeWidth; w++)
            {
                float x,y;
                x = (w + 0.5)/chromaframeWidth;  //center of pixel
                y = (h + 0.5)/chromaframeHeight;  //center of pixel

                //Load extra coordinates dependent on subsampling
                for(int j = 0; (j < m_horizontalSubSampling); j++)
                {
                    // set x for the pixel
                    m_videoFrameDataPoints_Chroma.push_back(x);
                    // set y for the pixel
                    m_videoFrameDataPoints_Chroma.push_back(y);
                }
            }
        }
    }
}



