#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>
#include <QTimer>


class OpenGLContext
    : public QOpenGLWidget,
      public QOpenGLFunctions_3_2_Core
{
public:
    OpenGLContext(QWidget *parent);
    ~OpenGLContext();

    void debugContextVersion();
    void printGLErrorLog();
    void printLinkInfoLog(int prog);
    void printShaderInfoLog(int shader);
};
