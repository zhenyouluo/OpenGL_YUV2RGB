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
#include <Magick++.h>

#include <iostream>

#include "cameraparameterset.h"


GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_vertice_indices_Vbo(QOpenGLBuffer::IndexBuffer),
      m_xRot(0),
      m_yRot(0),
      m_zRot(0),
      m_program(0),
      m_texture_data(0)
{
    m_core = QCoreApplication::arguments().contains(QStringLiteral("--coreprofile"));
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    m_transparent = QCoreApplication::arguments().contains(QStringLiteral("--transparent"));
    if (m_transparent)
        setAttribute(Qt::WA_TranslucentBackground);
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

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void GLWidget::cleanup()
{
    makeCurrent();
    m_logoVbo.destroy();
    m_vertices_Vbo.destroy();
    m_texture_coordinates_Vbo.destroy();
    m_depth_Vbo.destroy();
    m_vertice_indices_Vbo.destroy();
    delete m_program;
    delete m_texture_data;
    m_program = 0;
    doneCurrent();
}

void GLWidget::cameraSelectionChanged(QItemSelection newCamera)
{
//    qDebug() << "tst" << newCamera.first();
///    ... how to acces it/ cast it/ whatever ...
///

    qDebug() << "tst" << newCamera.first();
///    ... how to acces it/ cast it/ whatever ...
///
    CameraParameterSet test = newCamera.indexes().first().data(Qt::UserRole).value<CameraParameterSet>();

//        QModelIndexList indexes = selection.indexes();
//        foreach(const QModelIndex &index, indexes) {
//            QListWidgetItem *item = LoadedFilesListWidget->item(index.row());
//            // ...
//        }

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
    m_projMatrixLoc = m_program->uniformLocation("projMatrix");
    m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");
    m_normalMatrixLoc = m_program->uniformLocation("normalMatrix");
    m_lightPosLoc = m_program->uniformLocation("lightPos");

    m_matMVP_Loc = m_program->uniformLocation("MVP");
    m_matK2_inv_Loc = m_program->uniformLocation("K2_inv");
    m_matProjectXY_Loc = m_program->uniformLocation("matProjectXY_from_ref_to_virt");
    m_matProjectZ_Loc = m_program->uniformLocation("matProjectZ_from_ref_to_virt");
    m_matProjectXY_world_Loc = m_program->uniformLocation("matProjectXY_from_ref_to_virt_world");
    m_matProjectZ_world_Loc = m_program->uniformLocation("matProjectZ_from_ref_to_virt_world");

    m_vertices_Loc = m_program->attributeLocation("vertexPosition_modelspace");
    m_texture_Loc = m_program->attributeLocation("vertexUV");
    m_depth_Loc = m_program->attributeLocation("depth");

    // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
    // implementations this is optional and support may not be present
    // at all. Nonetheless the below code works in all cases and makes
    // sure there is a VAO when one is needed.
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);


    // compute frame vertices and copy to buffer
    computeFrameVertices(m_frameWidth, m_frameHeight);
    std::cout << "nr of pixels:" << m_frameWidth * m_frameHeight << std::endl;
    std::cout << "nr of vertices:" << m_videoFrameTriangles_vertices.size() << std::endl;
    m_vertices_Vbo.create();
    m_vertices_Vbo.bind();
    m_vertices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertices_Vbo.allocate(&m_videoFrameTriangles_vertices[0], m_videoFrameTriangles_vertices.size()* sizeof(glm::vec3));

    // compute indices of vertices and copy to buffer
    computeFrameMesh(m_frameWidth, m_frameHeight);
    std::cout << "nr of triangles:" << m_videoFrameTriangles_indices.size() / 3 << std::endl;
    m_vertice_indices_Vbo.create();

//    m_vertices_Vbo.
    m_vertice_indices_Vbo.bind();
    m_vertice_indices_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertice_indices_Vbo.allocate(&m_videoFrameTriangles_indices[0], m_videoFrameTriangles_indices.size() * sizeof(unsigned int));

    // compute texture coordinates and copy to buffer
    computeTextureCoordinates(m_frameWidth, m_frameHeight);
    std::cout << "nr of uv coordinates produced:" << m_videoFrameTriangles_texture_uv.size() / 2 << std::endl;
    m_texture_coordinates_Vbo.create();
    m_texture_coordinates_Vbo.bind();
    m_texture_coordinates_Vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_texture_coordinates_Vbo.allocate(&m_videoFrameTriangles_texture_uv[0], m_videoFrameTriangles_texture_uv.size() * sizeof(float));

    loadVideoData();

    // depthmap data
    m_depth_Vbo.create();
    m_depth_Vbo.bind();

    QImage test  = QImage("Poznan_Blocks_d0_1920x1080_25_f1_padded.bmp");
//    test.constBits()
//    m_depth_Vbo.allocate(test.constBits(), test.size().width()*test.size().height() * sizeof(GLubyte));
    m_depth_Vbo.allocate(m_depth_data.constData(), m_depth_data.count() * sizeof(GLfloat));



    // Store the vertex attribute bindings for the program.
    setupVertexAttribs();

    // Our camera never changes in this example.
    m_camera.setToIdentity();
    m_camera.translate(0, 0, -1);

    // Light position is fixed.
    m_program->setUniformValue(m_lightPosLoc, QVector3D(0, 0, 70));

    m_program->release();
}

void GLWidget::setupVertexAttribs()
{
//    ...
//    m_logoVbo.bind();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//    f->glEnableVertexAttribArray(0);
//    f->glEnableVertexAttribArray(1);
//    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
//    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
//    m_logoVbo.release();


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

//    // 3rd attribute buffer : depthmap
//    m_depth_Vbo.bind();
//    f->glEnableVertexAttribArray(m_depth_Loc);
//    f->glVertexAttribPointer(
//        m_depth_Loc,                      // attribute.
//        1,                                // size : 1
//        GL_UNSIGNED_BYTE,                         // type
//        GL_FALSE,                         // normalized?
//        0,                                // stride
//        (void*)0                          // array buffer offset
//    );


    // 3rd attribute buffer : depthmap
    m_depth_Vbo.bind();
    f->glEnableVertexAttribArray(m_depth_Loc);
    f->glVertexAttribPointer(
        m_depth_Loc,                      // attribute.
        1,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

}

void GLWidget::setupMatrices()
{
    glm::mat3x4 P2t(0.8948549867753071,   -0.05046163984489068, 411.5970167968247, -9060.076341977459,
                 0.01206148241001237,   0.9999514240487639, -43.61722592969886, -186.4324231963577,
                -9.195396620947882e-05, 6.830175425923271e-06, 1.070481143014745, 0.5386813524799995);

    m_P2 = glm::transpose(P2t);
    std::cout << "P2: " << glm::to_string(m_P2) << std::endl;

    m_mat3ProjectXY_from_ref_to_virt[0] = glm::vec3(m_P2[0]);
    m_mat3ProjectXY_from_ref_to_virt[1] = glm::vec3(m_P2[1]);
    m_mat3ProjectXY_from_ref_to_virt[2] = glm::vec3(m_P2[2]);
    std::cout << "mat3ProjectXY_from_ref_to_virt: " << glm::to_string(m_mat3ProjectXY_from_ref_to_virt) << std::endl;

    m_vec3ProjectZ_from_ref_to_virt = m_P2[3];
    std::cout << "vec3ProjectZ_from_ref_to_virt: " << glm::to_string(m_vec3ProjectZ_from_ref_to_virt) << std::endl;

    glm::mat4x4 P3t( 0.000569053341143,  -0.000033009518064,  -0.360748429344014,  -5.541990934259999,
                    0.000033645245483,   0.000576968745365,  -0.335637862771796,  -0.264130025039997,
                    -0.000091953966209,   0.000006830175426,   1.070481143014745,   0.538681352479999,
                    0,  -0.000000000000000,   0.000000000000000,   1.000000000000000);
    m_P3 = glm::transpose(P3t);
    std::cout << "P3: " << glm::to_string(m_P3) << std::endl;

    m_mat3x4ProjectXY_from_ref_to_virt_world[0] = glm::vec4(m_P3[0]);
    m_mat3x4ProjectXY_from_ref_to_virt_world[1] = glm::vec4(m_P3[1]);
    m_mat3x4ProjectXY_from_ref_to_virt_world[2] = glm::vec4(m_P3[2]);
    std::cout << "mat3x4ProjectXY_from_ref_to_virt_world: " << glm::to_string(m_mat3x4ProjectXY_from_ref_to_virt_world) << std::endl;

    m_vec4ProjectZ_from_ref_to_virt_world = glm::vec4(m_P3[3]) ;
    std::cout << "vec4ProjectZ_from_ref_to_virt_world: " << glm::to_string(m_vec4ProjectZ_from_ref_to_virt_world) << std::endl;

//    vec3 vertexPosition_modelspace;
//    vec4 virt_position_world = vec4(matProjectXY_from_ref_to_virt_world * vec3(vertexPosition_modelspace[0],vertexPosition_modelspace[1],1));

    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units


// get opengl projection matrix from calibration matrix of camera, have to assume near and far clipping plane distances n,f
//    K2: calibration matrix of second camera
//    [1728.807356436687, 0, 967.099291776573;
//      0, 1727.183523297295, 500.7943988015505;
//      0, 0, 1]

    glm::mat3 K2t(  1728.807356436687,  0,                  967.099291776573,
                    0,                  1727.183523297295,  500.7943988015505,
                    0,                  0,                  1);
    m_K2     = glm::transpose(K2t);
    std::cout << "K2: " << glm::to_string(m_K2) << std::endl;
    std::cout << "det K2: " << glm::determinant(m_K2) << std::endl;
    m_K2_inv = glm::inverse(m_K2);
    std::cout << "K2_inv: " << glm::to_string(m_K2_inv) << std::endl;

    float n = 0.0001f;
    float f = 100000.f;

    m_Projection = getProjectionFromCamCalibration(m_K2,f,n);

    std::cout << "Projection: " << glm::to_string(m_Projection) << std::endl;

//    glm::mat4
//            Projection = glm::perspective(45.0f, 16.0f / 9.0f, 0.1f, 500.0f);
    // Camera matrix
//position: -0.067845 0.459467 5.73404
//position: -10.8117 -5.4518 926.9588
//position+direction: -0.00739411 0.47914 4.73606


//    matWCoordFrame2CrefCoordFrame
//    [0.466974, -0.484961, 0.739424, 17.95368067746;
//      -0.763243, -0.643311, 0.060092, -28.04061200532;
//      0.446537, -0.592422, -0.670552, 8.320918414620001;
//      0, 0, 0, 1]
    glm::mat4 matWCoordFrame2CrefCoordFrame_t(
                0.466974, -0.484961, 0.739424, 17.95368067746,
                -0.763243, -0.643311, 0.060092, -28.04061200532,
                0.446537, -0.592422, -0.670552, 8.320918414620001,
                0,         0,          0,       1);
    m_matWCoordFrame2CrefCoordFrame = glm::transpose(matWCoordFrame2CrefCoordFrame_t);

//    glm::mat4 View       = m_matWCoordFrame2CrefCoordFrame;
    glm::mat4 View       = glm::lookAt(
                                glm::vec3(0,0,-3), // Camera position in World Space
                                glm::vec3(0,0,0), // looks at
                                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                           );
//     looks good from here:
//    position: -0.250721-0.486483-0.499824
//    position+direction: -0.322914-0.4880520.497565
//    glm::mat4 View       = glm::lookAt(
//                                glm::vec3(-0.250721,-0.486483,-0.499824), // Camera is at (4,3,-3), in World Space
//                                glm::vec3(-0.322914,-0.488052,0.497565), // and looks at the origin
//                                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
//                           );
//    glm::mat4 View       = glm::lookAt(
//                                glm::vec3(width/2,height/2,2000), // Camera is at (4,3,-3), in World Space
//                                glm::vec3(width/2,height/2,0), // and looks at the origin
//                                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
//                           );
    // Model matrix : an identity matrix (model will be at the origin)
    m_Model     = glm::mat4(1.0f);
//    Model               = glm::scale(Model, glm::vec3(0.5f,0.2f,2.f));
//    Model               = glm::rotate(Model, 30.f, glm::vec3(0.0f,1.f,0.f));
//    Model               = glm::rotate(Model, 30.f, glm::vec3(0.0f,0.f,1.f));
//    Model               = glm::translate(Model, glm::vec3(0.0f,1.f,-1.f));


    // Our ModelViewProjection : multiplication of our 3 matrices
    m_MVP        = m_Projection * View * m_Model; // Remember, matrix multiplication is the other way around
//    m_MVP        = m_Projection;
//    glm::mat4 MVP        = Projection; // Remember, matrix multiplication is the other way around
    std::cout << "Model: " << glm::to_string(m_Model) << std::endl;
    std::cout << "View: " << glm::to_string(View) << std::endl;
    std::cout << "Projection: " << glm::to_string(m_Projection) << std::endl;
    std::cout << "MVP: " << glm::to_string(m_MVP) << std::endl;
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);


    m_texture_data->bind();
    // Set our "myTextureSampler" sampler to user Texture Unit 0
//    glUniform1i(TextureID, 0);
    // Use texture unit 0 which contains cube.png
    m_program->setUniformValue("textureSampler", 0);


    m_world.setToIdentity();
    m_world.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_world.rotate(m_zRot / 16.0f, 0, 0, 1);


//    glm::mat3x3 mat_out;
//    glGetUniformfv(programID, K2_inv_ID, &mat_out[0][0]);
//    std::cout << "mat_out: " << glm::to_string(mat_out) << std::endl;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();
    m_program->setUniformValue(m_projMatrixLoc, m_proj);
    m_program->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
    QMatrix3x3 normalMatrix = m_world.normalMatrix();
    m_program->setUniformValue(m_normalMatrixLoc, normalMatrix);

//    m_program->setUniformValue(m_matMVP_Loc,&m_MVP[0][0]);
//    m_program->setUniformValue(m_matK2_inv_Loc,m_K2_inv);
//    m_program->setUniformValue(m_matProjectXY_Loc,m_mat3ProjectXY_from_ref_to_virt);
//    m_program->setUniformValue(m_matProjectZ_Loc,m_vec3ProjectZ_from_ref_to_virt);
//    m_program->setUniformValue(m_matProjectXY_world_Loc,m_mat3x4ProjectXY_from_ref_to_virt_world);
//    m_program->setUniformValue(m_matProjectZ_world_Loc,m_vec4ProjectZ_from_ref_to_virt_world);
    glUniformMatrix4fv(m_matMVP_Loc, 1, GL_FALSE, &m_MVP[0][0]);
    glUniformMatrix3fv(m_matK2_inv_Loc, 1, GL_FALSE, &m_K2_inv[0][0]);
    glUniformMatrix3fv(m_matProjectXY_Loc, 1, GL_FALSE, &m_mat3ProjectXY_from_ref_to_virt[0][0]);
    glUniform3fv(m_matProjectZ_Loc, 1, &m_vec3ProjectZ_from_ref_to_virt[0]);
//    glUniformMatrix3x4fv(m_matProjectXY_world_Loc, 1, GL_FALSE, &m_mat3x4ProjectXY_from_ref_to_virt_world[0][0]);
//    glUniform4fv(m_matProjectZ_world_Loc, 1, &m_vec4ProjectZ_from_ref_to_virt_world[0]);

    m_vertice_indices_Vbo.bind();

//    glDrawArrays(GL_TRIANGLES, 0, m_logo.vertexCount());
    glDrawElements(
        GL_TRIANGLES,      // mode
        m_videoFrameTriangles_indices.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    m_program->release();

    // Index buffer
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);


    // Return to onscreen rendering:
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER_EXT, 0);
//    glBindFramebuffer(1,0);

//    // Draw the triangles onscreen!
//    glDrawElements(
//        GL_TRIANGLES,      // mode
//        m_videoFrameTriangles_indices.size(),    // count
//        GL_UNSIGNED_INT,   // type
//        (void*)0           // element array buffer offset
//    );


//    // Bind our texture in Texture Unit 0
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, Texture);
//    // Set our "myTextureSampler" sampler to user Texture Unit 0
//    glUniform1i(TextureID, 0);

//    // 1rst attribute buffer : vertices
//    glEnableVertexAttribArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//    glVertexAttribPointer(
//        0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
//        3,                  // size
//        GL_FLOAT,           // type
//        GL_FALSE,           // normalized?
//        0,                  // stride
//        (void*)0            // array buffer offset
//    );

//    // 2nd attribute buffer : UVs
//    glEnableVertexAttribArray(1);
//    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
//    glVertexAttribPointer(
//        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
//        2,                                // size : U+V => 2
//        GL_FLOAT,                         // type
//        GL_FALSE,                         // normalized?
//        0,                                // stride
//        (void*)0                          // array buffer offset
//    );

//    // 3rd attribute buffer : depthmap
//    glEnableVertexAttribArray(2);
//    glBindBuffer(GL_ARRAY_BUFFER, depthmapbuffer);
//    glVertexAttribPointer(
//        2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
//        1,                                // size : U+V => 2
//        GL_FLOAT,                         // type
//        GL_FALSE,                         // normalized?
//        0,                                // stride
//        (void*)0                          // array buffer offset
//    );

//    // Index buffer
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);



//    //Before drawing
//    glBindFramebuffer(1, framebufferobject );

//    // Draw the triangles to offscreen rendering buffer
//    glDrawElements(
//        GL_TRIANGLES,      // mode
//        videoFrameTriangles_indices.size(),    // count
//        GL_UNSIGNED_INT,   // type
//        (void*)0           // element array buffer offset
//    );

//    //after drawing
//    glReadBuffer(GL_COLOR_ATTACHMENT0);
//    glReadPixels(0,0,width,height,GL_RGB,GL_FLOAT,&frameData[0]);

//    // Return to onscreen rendering:
////        glBindFramebuffer(GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER_EXT, 0);
//    glBindFramebuffer(1,0);

//    // Draw the triangles onscreen!
//    glDrawElements(
//        GL_TRIANGLES,      // mode
//        videoFrameTriangles_indices.size(),    // count
//        GL_UNSIGNED_INT,   // type
//        (void*)0           // element array buffer offset
//    );

//    glDisableVertexAttribArray(0);
//    glDisableVertexAttribArray(1);
//    glDisableVertexAttribArray(2);

//    // Swap buffers
//    glfwSwapBuffers(window);
//    glfwPollEvents();



}

void GLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(m_xRot + 8 * dy);
        setYRotation(m_yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(m_xRot + 8 * dy);
        setZRotation(m_zRot + 8 * dx);
    }
    m_lastPos = event->pos();
}

glm::mat4 GLWidget::getProjectionFromCamCalibration(glm::mat3 &calibrationMatrix, float clipFar, float clipNear)
{
    glm::mat4 Projection = glm::mat4(0.0f);
    Projection[0][0] = calibrationMatrix[0][0]/calibrationMatrix[2][0];
    Projection[1][1] = calibrationMatrix[1][1]/calibrationMatrix[2][1];
    Projection[2][2] = -(clipFar+clipNear)/(clipFar-clipNear);
    Projection[3][2] = -2*(clipFar*clipNear)/(clipFar-clipNear);
    Projection[2][3] = -1.f;

    return Projection;
}

void GLWidget::loadVideoData(){


    // Load the texture using any two methods
//    loadTexture_from_BMP("Poznan_Blocks_t0_1920x1080_25_f1.bmp");

    loadDepthMap("Poznan_Blocks_d0_1920x1080_25_f1_padded.bmp", m_frameWidth+2, m_frameHeight+2);

    // Load cube.png image
    m_texture_data = new QOpenGLTexture(QImage("Poznan_Blocks_t0_1920x1080_25_f1.bmp").mirrored());

    // Set nearest filtering mode for texture minification
    m_texture_data->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    m_texture_data->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    m_texture_data->setWrapMode(QOpenGLTexture::Repeat);

}


// function to generate the vertices corresponding to the pixels a frame. Each vertex is centered at a pixel, borders are padded.
void GLWidget::computeFrameVertices(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_vertices.clear();

    for( int h = -1; h < frameHeight + 1; h++)
    {
        for(int w = -1; w < frameWidth + 1; w++)
        {
            m_videoFrameTriangles_vertices.push_back(glm::vec3(w,h,0));
    //                std::cout << w << " " <<  h << " "<< 0 << " "<< std::endl;

        }
    }
}

// function to create a mesh by connecting the vertices to triangles
void GLWidget::computeFrameMesh(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_indices.clear();

    for( int h = 0; h <= frameHeight; h++)
    {
        for(int w = 0; w <= frameWidth; w++)
        {
            //tblr: top bottom left right
            int index_pixel_tl = h*(frameWidth+2) + w;
            int index_pixel_tr = index_pixel_tl + 1;
            int index_pixel_bl = (h+1)*(frameWidth+2) + w;
            int index_pixel_br = index_pixel_bl + 1;

            // each pixel generates two triangles, specify them so orientation is counter clockwise
            // first triangle
            m_videoFrameTriangles_indices.push_back(index_pixel_tl);
            m_videoFrameTriangles_indices.push_back(index_pixel_bl);
            m_videoFrameTriangles_indices.push_back(index_pixel_tr);
//                    std::cout << index_pixel_tl << " " <<  index_pixel_bl << " "<< index_pixel_tr << " "<< std::endl;
            // second triangle
            m_videoFrameTriangles_indices.push_back(index_pixel_bl);
            m_videoFrameTriangles_indices.push_back(index_pixel_br);
            m_videoFrameTriangles_indices.push_back(index_pixel_tr);
//                    std::cout << index_pixel_bl << " " <<  index_pixel_br << " "<< index_pixel_tr << " "<< std::endl;
        }
    }
}

// Function to fill Vector which will hold the texture coordinates which maps the texture (the video data) to the frame's vertices.
void GLWidget::computeTextureCoordinates(int frameWidth, int frameHeight)
{
    m_videoFrameTriangles_texture_uv.clear();

    for( int h = -1; h < frameHeight + 1; h++)
    {
        for(int w = -1; w < frameWidth + 1; w++)
        {
            float u,v;

            if(w == -1)
            //padding left border
            {
                u = 0.f;
            }
            else if(w == frameWidth)
            //padding right border
            {
                u = 1.f;
            }
            else
            //in the image
            {
                u = (w + 0.5)/frameWidth;  //center of pixel
            }

            if(h == -1)
            //padding upper border
            {
                v = 0.f;
            }
            else if(h == frameHeight)
            //padding lower border
            {
                v = 1.f;
            }
            else
            //in the image
            {
                v = (h + 0.5)/frameHeight;  //center of pixel
            }

    //                std::cout << "u: " << u <<  "v: " << v << std::endl;

            // set u for the pixel
            m_videoFrameTriangles_texture_uv.push_back(u);
            // set v for the pixel
            m_videoFrameTriangles_texture_uv.push_back(v);
        }
    }
}


void GLWidget::loadDepthMap(const char * imagepath, int frameWidth, int frameHeight){

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

        std::cout << image.magick() << std::endl;

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
        std::cout << "Caught exception: " << error_.what() << std::endl;
//        return 1;
    }
//    return 0;



    std::cout << "Read " << imageSize << " depth values" << std::endl;
//    std::cout << "With padding: " << imageSize << " depth values" << max << std::endl;
//    std::cout << "min: " << min << " max: " << max << std::endl;

}
