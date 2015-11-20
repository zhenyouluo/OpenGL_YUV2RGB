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
      m_texture_red_data(0),
      m_texture_green_data(0),
      m_texture_blue_data(0),
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
    m_texture_coordinates_Vbo.destroy();
    m_vertice_indices_Vbo.destroy();
    delete m_program;
//    delete m_texture_data;
    m_program = 0;
    doneCurrent();
}

void GLWidget::updateFrame(const QByteArray &textureData)
{
    QElapsedTimer timer;
    timer.start();

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    const int componentWidth = m_frameWidth;
    const int componentHeight = m_frameHeight;
    const int componentLength = componentWidth*componentHeight; // number of bytes per luma frames
    const unsigned char *srcY = (unsigned char*)textureData.data();
    const unsigned char *srcU = srcY + componentLength;
    const unsigned char *srcV = srcY + 2*componentLength;

    // transmitting the YUV data as three different textures
    // Y on unit 0
    m_texture_red_data = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_texture_red_data->create();
    m_texture_red_data->setSize(m_frameWidth,m_frameHeight);
    m_texture_red_data->setFormat(QOpenGLTexture::R8_UNorm);
    m_texture_red_data->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    m_texture_red_data->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, srcY);
    // Set filtering modes for texture minification and  magnification
    m_texture_red_data->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture_red_data->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    m_texture_red_data->setWrapMode(QOpenGLTexture::Repeat);

    // U on unit 1
    m_texture_green_data = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_texture_green_data->create();
    m_texture_green_data->setSize(m_frameWidth,m_frameHeight);
    m_texture_green_data->setFormat(QOpenGLTexture::R8_UNorm);
    m_texture_green_data->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    m_texture_green_data->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, srcU);
    // Set filtering modes for texture minification and  magnification
    m_texture_green_data->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture_green_data->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    m_texture_green_data->setWrapMode(QOpenGLTexture::Repeat);

    // V on unit 2
    m_texture_blue_data = std::make_shared<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_texture_blue_data->create();
    m_texture_blue_data->setSize(m_frameWidth,m_frameHeight);
    m_texture_blue_data->setFormat(QOpenGLTexture::R8_UNorm);
    m_texture_blue_data->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
    m_texture_blue_data->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, srcV);
    // Set filtering modes for texture minification and  magnification
    m_texture_blue_data->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture_blue_data->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    m_texture_blue_data->setWrapMode(QOpenGLTexture::Repeat);

    qDebug() << "Moving data to graphics card took" << timer.elapsed() << "milliseconds";

    update();
}


void GLWidget::updateFormat(int frameWidth, int frameHeight)
{
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_textureFormat = QImage::Format_RGB32;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    // just put video on two triangles forming a rectangle
    // compute frame vertices and copy to buffer
    m_videoFrameTriangles_vertices.clear();
    m_videoFrameTriangles_vertices.push_back(glm::vec3(0,0,0));
    m_videoFrameTriangles_vertices.push_back(glm::vec3(m_frameWidth,0,0));
    m_videoFrameTriangles_vertices.push_back(glm::vec3(0,m_frameHeight,0));
    m_videoFrameTriangles_vertices.push_back(glm::vec3(m_frameWidth,m_frameHeight,0));
    m_videoFrameTriangles_indices.push_back(0);
    m_videoFrameTriangles_indices.push_back(2);
    m_videoFrameTriangles_indices.push_back(1);
    m_videoFrameTriangles_indices.push_back(1);
    m_videoFrameTriangles_indices.push_back(2);
    m_videoFrameTriangles_indices.push_back(3);
    m_videoFrameTriangles_texture_uv.clear();
    m_videoFrameTriangles_texture_uv.push_back(0.f);
    m_videoFrameTriangles_texture_uv.push_back(0.f);
    m_videoFrameTriangles_texture_uv.push_back(1.f);
    m_videoFrameTriangles_texture_uv.push_back(0.f);
    m_videoFrameTriangles_texture_uv.push_back(0.f);
    m_videoFrameTriangles_texture_uv.push_back(1.f);
    m_videoFrameTriangles_texture_uv.push_back(1.f);
    m_videoFrameTriangles_texture_uv.push_back(1.f);
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
    m_texture_coordinates_Vbo.create();
    m_texture_coordinates_Vbo.bind();
    m_texture_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_texture_coordinates_Vbo.allocate(&m_videoFrameTriangles_texture_uv[0], m_videoFrameTriangles_texture_uv.size() * sizeof(float));

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
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    setupMatrices();

    m_frameHeight = 1080;
    m_frameWidth = 1920;


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
    m_texture_Loc = m_program->attributeLocation("vertexUV");

    // Create vertex buffer objects
    m_vertices_Vbo.create();
    m_vertice_indices_Vbo.create();
    m_texture_coordinates_Vbo.create();

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
    m_texture_coordinates_Vbo.bind();
    f->glEnableVertexAttribArray(m_texture_Loc);
    f->glVertexAttribPointer(
        m_texture_Loc,                    // attribute.
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
    if(m_texture_red_data == NULL || m_texture_green_data == NULL || m_texture_blue_data == NULL) return;
//    if(m_texture_red_data == NULL ) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    m_texture_red_data->bind(0);
    m_texture_green_data->bind(1);
    m_texture_blue_data->bind(2);

//    m_program->setUniformValue("textureSamplerRed", 0);
//    m_program->setUniformValue("textureSamplerGreen", 1);
//    m_program->setUniformValue("textureSamplerBlue", 2);



    glUniformMatrix4fv(m_matMVP_Loc, 1, GL_FALSE, &m_MVP[0][0]);


    m_vertice_indices_Vbo.bind();
    m_vertices_Vbo.bind();
    m_texture_coordinates_Vbo.bind();

    glDrawElements(
        GL_TRIANGLES,      // mode
        m_videoFrameTriangles_indices.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    m_program->release();

//    qDebug() << "Painting took" << timer.elapsed() << "milliseconds";
    int msSinceLastPaint = m_measureFPSTimer.restart();
    emit msSinceLastPaintChanged(msSinceLastPaint);
}




