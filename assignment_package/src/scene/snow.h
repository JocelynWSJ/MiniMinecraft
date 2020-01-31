#pragma once
#ifndef SNOW_H
#define SNOW_H

#include "drawable.h"
#include <la.h>
#include "noise.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Snow : public Drawable
{
private:
    // left back bottom vertex
    glm::vec4 m_originPos;
    // pos
    std::vector<glm::vec4> m_pos;
    // normal of snow
    std::vector<glm::vec2> m_direction;
    // velocity of snow
    std::vector<glm::vec2> m_velocity;
    int m_type;
    void createRect(std::vector<glm::vec4>& verts,
                    std::vector<GLint>& idx,
                    glm::vec4& pos,
                    glm::vec2& direction,
                    glm::vec2& velocity);

public:
    Snow(OpenGLContext* context, glm::vec4 getOriginPos);

    virtual ~Snow(){}
    void create() override;
    glm::vec4 getOriginPos() const;
    void setOriginPos(const glm::vec4 &getOriginPos);

};

#endif // SNOW_H
