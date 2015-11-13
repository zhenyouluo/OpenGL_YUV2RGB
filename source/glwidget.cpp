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

#include "cameraparameterset.h"


GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),      
      m_frameWidth(1),
      m_frameHeight(1),
      m_vertice_indices_Vbo(QOpenGLBuffer::IndexBuffer),
      m_texture_data(0),
      m_program(0)
{
    m_core = QCoreApplication::arguments().contains(QStringLiteral("--coreprofile"));
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    m_transparent = QCoreApplication::arguments().contains(QStringLiteral("--transparent"));
    if (m_transparent)
        setAttribute(Qt::WA_TranslucentBackground);

    m_zNear = 15;
    m_zFar = 150;

    QSizePolicy p(sizePolicy());
//    p.setHorizontalPolicy(QSizePolicy::Fixed);
//    p.setVerticalPolicy(QSizePolicy::Fixed);
    p.setHeightForWidth(true);
    setSizePolicy(p);

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
    m_depth_Vbo.destroy();
    m_vertice_indices_Vbo.destroy();
    delete m_program;
//    delete m_texture_data;
    m_program = 0;
    doneCurrent();
}

void GLWidget::updateViewCamera(QItemSelection newCamera)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    if(newCamera.isEmpty()) return;

    CameraParameterSet camParams = newCamera.indexes().first().data(Qt::UserRole).value<CameraParameterSet>();

    m_K_view = camParams.getK();
    m_R_view = camParams.getR();
    m_t_view = camParams.getT();

    //recompute opengl matrices
    setupMatrices();

    // draw from new perspective
    update();
}

void GLWidget::updateZNear(float zNear)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    m_zNear = zNear;

    //recompute opengl matrices
    setupMatrices();

    // draw from new perspective
    update();

}

void GLWidget::updateZFar(float zFar)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    m_zFar = zFar;

    //recompute opengl matrices
    setupMatrices();

    // draw from new perspective
    update();
}

void GLWidget::updateFrame(const QImage &textureData, const QVector<uint8_t> &depthData)
{
    // Set new texture
//    QImage texture = QImage(reinterpret_cast<const unsigned char*>(textureData.constData()),m_frameWidth,m_frameHeight,m_textureFormat);
    m_texture_data = std::make_shared<QOpenGLTexture>(textureData,QOpenGLTexture::DontGenerateMipMaps);
    // Set nearest filtering mode for texture minification
    m_texture_data->setMinificationFilter(QOpenGLTexture::Nearest);
    // Set bilinear filtering mode for texture magnification
    m_texture_data->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    m_texture_data->setWrapMode(QOpenGLTexture::Repeat);

    // set new depth data
    m_depth_Vbo.bind();
    m_depth_Vbo.allocate(depthData.constData(), depthData.count() * sizeof(GL_UNSIGNED_BYTE));

    update();
}

void GLWidget::updateFrame(const QImage &textureData, const QVector<float> &depthData)
{
    // Set new texture
//    QImage texture = QImage(reinterpret_cast<const unsigned char*>(textureData.constData()),m_frameWidth,m_frameHeight,m_textureFormat);
    m_texture_data = std::make_shared<QOpenGLTexture>(textureData,QOpenGLTexture::DontGenerateMipMaps);
    // Set nearest filtering mode for texture minification
    m_texture_data->setMinificationFilter(QOpenGLTexture::Nearest);
    // Set bilinear filtering mode for texture magnification
    m_texture_data->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    m_texture_data->setWrapMode(QOpenGLTexture::Repeat);

    // set new depth data
    m_depth_Vbo.bind();
    m_depth_Vbo.allocate(depthData.constData(), depthData.count() * sizeof(GLfloat));

    update();
}

void GLWidget::updateFrame(const QImage &textureData, const QByteArray &depthData)
{
    QElapsedTimer timer;
    timer.start();
    // Set new texture
//    QImage texture = QImage(reinterpret_cast<const unsigned char*>(textureData.constData()),m_frameWidth,m_frameHeight,m_textureFormat);
    m_texture_data = std::make_shared<QOpenGLTexture>(textureData,QOpenGLTexture::DontGenerateMipMaps);
    // Set nearest filtering mode for texture minification
    m_texture_data->setMinificationFilter(QOpenGLTexture::Nearest);
    // Set bilinear filtering mode for texture magnification
    m_texture_data->setMagnificationFilter(QOpenGLTexture::Linear);
    // Wrap texture coordinates by repeating
    m_texture_data->setWrapMode(QOpenGLTexture::Repeat);

    // set new depth data
    m_depth_Vbo.bind();
    m_depth_Vbo.allocate(depthData.constData(), depthData.count() * sizeof(GLbyte));
//    m_depth_Vbo.allocate(depthDataVec.constData(), depthDataVec.count() * sizeof(GLfloat));

    qDebug() << "Moving data to graphics card took" << timer.elapsed() << "milliseconds";

    update();
}


void GLWidget::updateFormat(int frameWidth, int frameHeight)
{
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_textureFormat = QImage::Format_RGB32;

    // compute frame vertices and copy to buffer
    computeFrameVertices(m_frameWidth, m_frameHeight);
    std::cout << "nr of pixels:" << m_frameWidth * m_frameHeight << std::endl;
    std::cout << "nr of vertices:" << m_videoFrameTriangles_vertices.size() << std::endl;

    m_vertices_Vbo.bind();
    m_vertices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertices_Vbo.allocate(&m_videoFrameTriangles_vertices[0], m_videoFrameTriangles_vertices.size()* sizeof(glm::vec3));

    // compute indices of vertices and copy to buffer
    computeFrameMesh(m_frameWidth, m_frameHeight);
    std::cout << "nr of triangles:" << m_videoFrameTriangles_indices.size() / 3 << std::endl;

    m_vertice_indices_Vbo.bind();
    m_vertice_indices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertice_indices_Vbo.allocate(&m_videoFrameTriangles_indices[0], m_videoFrameTriangles_indices.size() * sizeof(unsigned int));

    // compute texture coordinates and copy to buffer
    computeTextureCoordinates(m_frameWidth, m_frameHeight);
    std::cout << "nr of uv coordinates produced:" << m_videoFrameTriangles_texture_uv.size() / 2 << std::endl;
    m_texture_coordinates_Vbo.bind();
    m_texture_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_texture_coordinates_Vbo.allocate(&m_videoFrameTriangles_texture_uv[0], m_videoFrameTriangles_texture_uv.size() * sizeof(float));


//    // very simple model for debugging: just put video on two triangles forming a rectangle
//    // compute frame vertices and copy to buffer
//    m_videoFrameTriangles_vertices.clear();
//    m_videoFrameTriangles_vertices.push_back(glm::vec3(0,0,0));
//    m_videoFrameTriangles_vertices.push_back(glm::vec3(m_frameWidth,0,0));
//    m_videoFrameTriangles_vertices.push_back(glm::vec3(0,m_frameHeight,0));
//    m_videoFrameTriangles_vertices.push_back(glm::vec3(m_frameWidth,m_frameHeight,0));
//    m_videoFrameTriangles_indices.push_back(0);
//    m_videoFrameTriangles_indices.push_back(2);
//    m_videoFrameTriangles_indices.push_back(1);
//    m_videoFrameTriangles_indices.push_back(1);
//    m_videoFrameTriangles_indices.push_back(2);
//    m_videoFrameTriangles_indices.push_back(3);
//    m_videoFrameTriangles_texture_uv.clear();
//    m_videoFrameTriangles_texture_uv.push_back(0.f);
//    m_videoFrameTriangles_texture_uv.push_back(0.f);
//    m_videoFrameTriangles_texture_uv.push_back(1.f);
//    m_videoFrameTriangles_texture_uv.push_back(0.f);
//    m_videoFrameTriangles_texture_uv.push_back(0.f);
//    m_videoFrameTriangles_texture_uv.push_back(1.f);
//    m_videoFrameTriangles_texture_uv.push_back(1.f);
//    m_videoFrameTriangles_texture_uv.push_back(1.f);
//    m_vertices_Vbo.create();
//    m_vertices_Vbo.bind();
//    m_vertices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
//    m_vertices_Vbo.allocate(&m_videoFrameTriangles_vertices[0], m_videoFrameTriangles_vertices.size()* sizeof(glm::vec3));
//    // compute indices of vertices and copy to buffer
//    std::cout << "nr of triangles:" << m_videoFrameTriangles_indices.size() / 3 << std::endl;
//    m_vertice_indices_Vbo.create();
//    m_vertice_indices_Vbo.bind();
//    m_vertice_indices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
//    m_vertice_indices_Vbo.allocate(&m_videoFrameTriangles_indices[0], m_videoFrameTriangles_indices.size() * sizeof(unsigned int));
//    m_texture_coordinates_Vbo.create();
//    m_texture_coordinates_Vbo.bind();
//    m_texture_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
//    m_texture_coordinates_Vbo.allocate(&m_videoFrameTriangles_texture_uv[0], m_videoFrameTriangles_texture_uv.size() * sizeof(float));


}

void GLWidget::updateReferenceCamera(CameraParameterSet refCam)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    m_K_ref = refCam.getK();
    m_R_ref = refCam.getR();
    m_t_ref = refCam.getT();

    //recompute opengl matrices
    setupMatrices();

    // draw from new perspective
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

    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex,":shaders/vertexshader.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment,":shaders/fragmentshader.glsl");
    m_program->link();
    m_program->bind();

    m_matMVP_Loc = m_program->uniformLocation("MVP");
    m_matK2_inv_Loc = m_program->uniformLocation("K2_inv");

    m_vertices_Loc = m_program->attributeLocation("vertexPosition_modelspace");
    m_texture_Loc = m_program->attributeLocation("vertexUV");
    m_depth_Loc = m_program->attributeLocation("depth");

    // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
    // implementations this is optional and support may not be present
    // at all. Nonetheless the below code works in all cases and makes
    // sure there is a VAO when one is needed.
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);


    // Create vertex buffer objects
    m_vertices_Vbo.create();
    m_vertice_indices_Vbo.create();
    m_texture_coordinates_Vbo.create();
    m_depth_Vbo.create();

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

    // 3rd attribute buffer : depthmap
    m_depth_Vbo.bind();
    f->glEnableVertexAttribArray(m_depth_Loc);
    f->glVertexAttribPointer(
        m_depth_Loc,                      // attribute.
        1,                                // size : 1
        GL_UNSIGNED_BYTE,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );


//    // 3rd attribute buffer : depthmap
//    m_depth_Vbo.bind();
//    f->glEnableVertexAttribArray(m_depth_Loc);
//    f->glVertexAttribPointer(
//        m_depth_Loc,                      // attribute.
//        1,                                // size : U+V => 2
//        GL_FLOAT,                         // type
//        GL_FALSE,                         // normalized?
//        0,                                // stride
//        (void*)0                          // array buffer offset
//    );

}

void GLWidget::setupMatrices()
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    m_Kinv_Cref = glm::inverse(m_K_ref);

    // construct transform from world to reference coordinates
    // put R_Cref in upper left 3x3 part of matrix, then set 4th column to [t_Cref 1], all else 0
    glm::mat4 worldCoords2CrefCoords(m_R_ref);
    worldCoords2CrefCoords[3] = glm::vec4(m_t_ref, 1.f);

    // construct transform from world to reference coordinates, analog to worldCoords2CrefCoords
    glm::mat4 worldCoords2CvirtCoords(m_R_view);
    worldCoords2CvirtCoords[3] = glm::vec4(m_t_view, 1.f);

    // transform matrix from ref view to virtual view
    m_P_moveFromReferenceToVirtualView = worldCoords2CvirtCoords * glm::inverse(worldCoords2CrefCoords);

    m_K_projectVirtualViewToImage = getProjectionFromCamCalibration(m_K_view,m_zFar,m_zNear);

    // Our ModelViewProjection : projection of model to different view and then to image
    m_MVP        = m_K_projectVirtualViewToImage * m_P_moveFromReferenceToVirtualView;

    //    std::cout << "det of R_Cref: " << glm::determinant(m_R_ref) << std::endl;
    //    std::cout << "K2_inv: " << glm::to_string(m_Kinv_Cref) << std::endl;
    //    std::cout << "K_Cref: " << glm::to_string(m_K_ref) << std::endl;
    //    std::cout << "m_K_view: " << glm::to_string(m_K_view) << std::endl;
    //    std::cout << "R_Cref: " << glm::to_string(m_R_ref) << std::endl;
    //    std::cout << "m_R_view: " << glm::to_string(m_R_view) << std::endl;
    //    std::cout << "t_Cref: " << glm::to_string(m_t_ref) << std::endl;
    //    std::cout << "m_t_view: " << glm::to_string(m_t_view) << std::endl;
    //    std::cout << "worldCoords2CrefCoords: " << glm::to_string(worldCoords2CrefCoords) << std::endl;
    //    std::cout << "worldCoords2CvirtCoords: " << glm::to_string(worldCoords2CvirtCoords) << std::endl;
    //    std::cout << "m_P_moveFromReferenceToVirtualView: " << glm::to_string(m_P_moveFromReferenceToVirtualView) << std::endl;
    //    std::cout << "MVP: " << glm::to_string(m_MVP) << std::endl;
}

void GLWidget::paintGL()
{
    qDebug() << "Function Name: " << Q_FUNC_INFO;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CCW);

    // nothing loaded yet
    if(m_texture_data == NULL) return;

    m_texture_data->bind();
    m_program->setUniformValue("textureSampler", 0);

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    glUniformMatrix4fv(m_matMVP_Loc, 1, GL_FALSE, &m_MVP[0][0]);
    glUniformMatrix3fv(m_matK2_inv_Loc, 1, GL_FALSE, &m_Kinv_Cref[0][0]);
    m_program->setUniformValue("zNear",m_zNear);
    m_program->setUniformValue("zFar",m_zFar);

    m_vertice_indices_Vbo.bind();

    glDrawElements(
        GL_TRIANGLES,      // mode
        m_videoFrameTriangles_indices.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    m_program->release();

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


// function to generate the vertices corresponding to the pixels of a frame. Each vertex is centered at a pixel, borders are padded.
void GLWidget::computeFrameVertices(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_vertices.clear();

    for( int h = 0; h < frameHeight; h++)
    {
        for(int w = 0; w < frameWidth; w++)
        {

            m_videoFrameTriangles_vertices.push_back(glm::vec3(w,h,0));
        }
    }
}

// function to create a mesh by connecting the vertices to triangles
void GLWidget::computeFrameMesh(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_indices.clear();

    for( int h = 0; h < frameHeight; h++)
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
    }
}

// Function to fill Vector which will hold the texture coordinates which maps the texture (the video data) to the frame's vertices.
void GLWidget::computeTextureCoordinates(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_texture_uv.clear();

    for( int h = 0; h < frameHeight; h++)
    {
        for(int w = 0; w < frameWidth; w++)
        {
            float u,v;

            u = (w + 0.5)/frameWidth;  //center of pixel
            v = (h + 0.5)/frameHeight;  //center of pixel

            // set u for the pixel
            m_videoFrameTriangles_texture_uv.push_back(u);
            // set v for the pixel
            m_videoFrameTriangles_texture_uv.push_back(v);
        }
    }
}

