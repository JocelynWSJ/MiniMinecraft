#pragma once

#include "drawable.h"
#include <la.h>
#include "noise.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class RainDrop : public Drawable
{
private:
    // left back bottom vertex
    glm::vec4 m_originPos;
    // color of rain
    glm::vec4 m_color;
    // pos
    std::vector<glm::vec4> m_pos;
    // the normals of the rain
    std::vector<glm::vec2> m_direction;
    // time offset
    std::vector<float> m_offset;
    // bounced height
    std::vector<float> m_height;
    bool shouldBounced;
    void createRect(std::vector<glm::vec4>& verts,
                    std::vector<GLint>& idx,
                    glm::vec4& pos,
                    glm::vec2& direction,
                    float offset, float height);

public:
    RainDrop(OpenGLContext* context, glm::vec4 getOriginPos,
         glm::vec4 color);

    virtual ~RainDrop(){}
    void create() override;
    glm::vec4 getOriginPos() const;
    void setOriginPos(const glm::vec4 &getOriginPos);
    float getOffset(int x, int z) const;
    void setOffset(int x, int z, float offset);
    float getHeight(int x, int z) const;
    void setHeight(int x, int z, float height);
    bool getShouldBounced() const;
    void setShouldBounced(bool value);
};
