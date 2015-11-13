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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QMatrix3x3>
#include <QMatrix3x4>
#include <QMatrix4x3>
#include <QGenericMatrix>
#include <memory>
#include <QByteArray>
#include <QVector>

// Include GLM
#include <glm/glm.hpp>
#include <QItemSelection>

#include "cameraparameterset.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    int heightForWidth(int w) const;

public slots:
    void cleanup();
    void updateViewCamera(QItemSelection newCamera);
    void updateFrame(const QImage &textureData, const QVector<float> &depthData); // display a new frame
    void updateFrame(const QImage &textureData, const QByteArray &depthData); // display a new frame
    void updateFrame(const QImage &textureData, const QVector<uint8_t> &depthData);
    void updateFormat(int frameWidth, int frameHeight); // change frame format (width, height, ...)
    void updateReferenceCamera(CameraParameterSet refCam);

signals:


protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    void computeFrameVertices(int frameWidth, int frameHeight);
    void computeFrameMesh(int frameWidth, int frameHeight);
    void computeTextureCoordinates(int frameWidth, int frameHeight);
    void setupVertexAttribs();
    void setupMatrices();
    glm::mat4 getProjectionFromCamCalibration(glm::mat3 &calibrationMatrix, float clipFar, float clipNear);

    bool m_core;

    int m_frameWidth;
    int m_frameHeight;

    QPoint m_lastPos;    
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vertices_Vbo;
    QOpenGLBuffer m_vertice_indices_Vbo;
    QOpenGLBuffer m_texture_coordinates_Vbo;
    QOpenGLBuffer m_depth_Vbo;

    std::shared_ptr<QOpenGLTexture> m_texture_data;
    QImage::Format m_textureFormat;
    QVector<GLfloat> m_vertices_data;
    QVector<GLfloat> m_depth_data;

    QOpenGLShaderProgram *m_program;

    int m_matMVP_Loc;
    int m_matK2_inv_Loc;

    // handles for texture, vertices and depth
    int m_vertices_Loc;
    int m_texture_Loc;
    int m_depth_Loc;

    glm::mat3 m_Kinv_Cref;    
    glm::mat4 m_MVP;

    glm::mat4 m_P_moveFromReferenceToVirtualView;
    glm::mat4 m_K_projectVirtualViewToImage;

    glm::mat3 m_K_ref;
    glm::mat3 m_R_ref;
    glm::vec3 m_t_ref;
    glm::mat3 m_K_view;
    glm::mat3 m_R_view;
    glm::vec3 m_t_view;

    std::vector<glm::vec3> m_videoFrameTriangles_vertices;
    // each vector (of 3 unsigned int) holds the indices for one triangle in the video frame
    std::vector<unsigned int> m_videoFrameTriangles_indices;
    // Vector which will the texture coordinates which maps the texture (the video data) to the frame's vertices.
    std::vector<float> m_videoFrameTriangles_texture_uv;

    bool m_transparent;

};

#endif
