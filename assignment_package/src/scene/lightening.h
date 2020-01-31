#pragma once
#ifndef LIGHTENING_H
#define LIGHTENING_H

#endif // LIGHTENING_H

#include "drawable.h"
#include <la.h>
#include "noise.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Lightening : public Drawable
{
private:
    // left back bottom vertex
    glm::vec4 m_originPos;
    void addFace(glm::vec4 pos0,
                 glm::vec4 pos1,
                 glm::vec4 nor,
                 std::vector<glm::vec4>& verts,
                 std::vector<GLuint>& idx) const;

public:
    Lightening(OpenGLContext* context, glm::vec4 pos) : Drawable(context), m_originPos(pos) {}

    virtual ~Lightening(){}
    void create() override;
};
