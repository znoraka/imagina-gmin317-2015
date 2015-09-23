/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
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

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>
#include <QtGlobal>
#include <QtGui/QImage>
#include <QCursor>

//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();
    static const int sizeX = 240;
    static const int sizeY = 240;


    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    GLuint loadShader(GLenum type, const char *source);
    GLfloat* initVertices(GLint countX, GLint countY);
    float getRandomZ(float i, float j);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_normalAttr;
    GLuint m_matrixUniform;
    QCursor* cursor;

    GLfloat* vertices;
    QImage image;

    QOpenGLShaderProgram *m_program;
    int m_frame;
    float n, x, y, a, b;
    int direction;
};

TriangleWindow::TriangleWindow()
    : m_program(0)
    , m_frame(0)
{
}

GLfloat *TriangleWindow::initVertices(GLint countX, GLint countY)
{
    int count = countX * countY * 3 * 2 + countX * 3 + 3;
    qDebug() << count;
    GLfloat *array = new GLfloat[count];
    GLfloat stepX = 1.0 / (countX);
    GLfloat stepY = 1.0 / (countY);
    int cpt = 0;

    float posX = -0.5f;
    float posY = -0.5f;

    int flop = 1;

    for (int i = 0; i < countX; ++i) {
        for (int j = 0; j < countY; ++j) {
            array[cpt++] = posX;
            array[cpt++] = posY;
            array[cpt++] = getRandomZ(posX, posY);

            array[cpt++] = posX + stepX;
            array[cpt++] = posY;
            array[cpt++] = getRandomZ(posX + stepX, posY);

            posY += stepY * flop;
        }

        array[cpt++] = posX;
        array[cpt++] = posY;
        array[cpt++] = getRandomZ(posX, posY);

        flop *= -1;
        posX += stepX;
    }

    array[cpt++] = posX;
    array[cpt++] = posY;
    array[cpt++] = getRandomZ(posX, posY);
    qDebug() << cpt;
    return array;
}

float TriangleWindow::getRandomZ(float i, float j)
{
    return qGray(this->image.pixel((this->sizeX * (i + 0.5f)), (this->sizeY * (j + 0.5f)))) * 0.0008f;
}

//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(800, 600);
    window.show();

    window.setAnimating(true);

    return app.exec();
}
//! [2]


//! [3]
static const char *vertexShaderSource =
        "attribute highp vec4 posAttr;\n"
        "attribute lowp vec4 colAttr;\n"
        "varying lowp vec4 col;\n"
        "uniform highp mat4 matrix;\n"
        "float rand(vec2 co){ \n"
        "return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
        "}\n"

        "void main() {\n"

        "	if(posAttr.z < 0.08 - rand(posAttr.xz) * 0.03) { \n"
        "	  col = vec4(0 + posAttr.z, 0.4, 0, 1) * (pow((1 + posAttr.z), 3) - 0.8) * 2; \n"
        "	} else if (posAttr.z > 0.08 - rand(posAttr.xz) * 0.03 && posAttr.z < 0.15) { \n"
        "	  col = vec4(0.54, 0.27 + posAttr.z, 0.07, 1) * (pow((1 + posAttr.z), 3) - 1); \n"
        "	} else if(posAttr.z > 0.15){ \n"
        "	  col = vec4(0.9, 0.9, 0.8, 1) * (pow((1 + posAttr.z), 3) - 1); \n"
        "	} \n"

        "   gl_Position = matrix * posAttr;\n"
        "}\n";

static const char *fragmentShaderSource =
        "varying lowp vec4 col;\n"
        "void main() {\n"
        "   gl_FragColor = col;\n"
        "}\n";
//! [3]

//! [4]
GLuint TriangleWindow::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    this->image = QImage("/home/noe/Documents/dev/imagina-gmin317-2015/tp1/heightmap-1.png");
    this->vertices = initVertices(sizeX, sizeY);
    n = 0;
    x = 0;
    y = -0.14f;
    a = 0;
    b = 0;
    this->direction = 0;
    this->cursor = new QCursor(Qt::BlankCursor);
    this->setCursor(*cursor);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);								// Ne traite pas les faces cachées

}
//! [4]

//! [5]
void TriangleWindow::render()
{
    this->cursor->setPos(this->position().x() + width() * 0.5f, this->position().y() + height() * 0.5f);

    qreal retinaScale = 16.0f/9.0f;
    glViewport(-width() * 0.5f, -height() * 0.5f, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();


    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 16.0f/9.0f, 0.01f, 10.0f);
    matrix.rotate(100.0f * n, 1, 0, 0);
    matrix.rotate(100.0f * x, 0, 0, 1);

    if(direction != 0) {
        a -= (matrix.data()[0]) * 0.001f * direction;
        b += (matrix.data()[4]) * 0.001f * direction;
    }

    matrix.translate(b, a, - getRandomZ(-b, -a) - 0.02f);

    m_program->setUniformValue(m_matrixUniform, matrix);

    glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeX * sizeY * 2 + sizeX + 1);
    glDisableVertexAttribArray(0);
    m_program->release();

    ++m_frame;

}

bool TriangleWindow::event(QEvent *event)
{
    QKeyEvent *keyEvent;
    QMouseEvent *mouseEvent;
    float deltaX = this->width() * 0.5f;
    float deltaY = this->height() * 0.5f;

    switch (event->type()) {
    case QEvent::MouseMove:
        mouseEvent = static_cast<QMouseEvent*>(event);
        this->x -= (deltaX - mouseEvent->x()) * 0.001f;
        this->n -= (deltaY - mouseEvent->y()) * 0.001f;
        return true;
    case QEvent::KeyPress:
        keyEvent = static_cast<QKeyEvent*>(event);

        if(keyEvent->key() == Qt::Key_Space) {
            y += 0.01f;
        } else if(keyEvent->key() == Qt::Key_Shift) {
            y -= 0.01f;
        } else if(keyEvent->key() == Qt::Key_Up) {
            direction = 1;
        } else if(keyEvent->key() == Qt::Key_Down) {
            direction = -1;
        } else if (keyEvent->key() == Qt::Key_Escape) {
            qApp->exit();
        }

        return true;
    case QEvent::KeyRelease:
        direction = 0;
        return true;
    }
    OpenGLWindow::event(event);
}
//! [5]
