#pragma once

#include "drawable.h"
#include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class WorldAxes : public Drawable
{
public:
    WorldAxes(OpenGLContext* context) : Drawable(context){}
    virtual ~WorldAxes(){}
    void create() override;
    GLenum drawMode() override;
};
