#ifndef QUAD_H
#define QUAD_H


#pragma once

#include "drawable.h"
#include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Quad : public Drawable
{
public:
    Quad(OpenGLContext* context);
    virtual void create();
};

#endif // QUAD_H
