#include "snow.h"
#include <la.h>
#include <iostream>

Snow::Snow(OpenGLContext* context, glm::vec4 pos)
    : Drawable(context),
      m_originPos(pos) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            m_pos.push_back(glm::vec4(m_originPos.x + i,
                                      m_originPos.y + (double)rand() / RAND_MAX * 100,
                                      m_originPos.z + j,
                                      1));
            m_direction.push_back(glm::vec2((double)rand() / RAND_MAX - 0.5, (double)rand()/ RAND_MAX - 0.5));
            m_velocity.push_back(glm::vec2(((double)rand() / RAND_MAX - 0.5) * 0.2, ((double)rand() / RAND_MAX - 0.5) * 0.05 + 0.05));
        }
    }
}

glm::vec4 Snow::getOriginPos() const
{
    return m_originPos;
}

void Snow::setOriginPos(const glm::vec4 &pos)
{
    m_originPos = pos;
}

void Snow::createRect(std::vector<glm::vec4>& verts,
                      std::vector<GLint>& idx,
                      glm::vec4& pos,
                      glm::vec2& direction,
                      glm::vec2& velocity) {
    int count = verts.size() / 4;
    idx.push_back(count);
    idx.push_back(count + 1);
    idx.push_back(count + 2);
    idx.push_back(count);
    idx.push_back(count + 2);
    idx.push_back(count + 3);

    glm::vec2 uv(0, 0);
    int type = int(pos.x + pos.z) % 2;
    switch (type) {
    case 0:
        uv.x = 8 + 1.0 / 16 * 10;
        uv.y = 5 + 1.0 / 16 * 10;
        break;
    case 1:
        uv.x = 8 + 1.0 / 16 * 11;
        uv.y = 5 + 1.0 / 16 * 12;
        break;
    default:
        break;
    }

    direction = glm::normalize(direction);
    glm::vec4 color(velocity.x, velocity.y, 0, 0);
    glm::vec4 nor(direction.y, 0, direction.x, 0);
    verts.push_back(pos);
    verts.push_back(nor);
    verts.push_back(color);
    verts.push_back(glm::vec4(uv.x / 16., uv.y / 16., 130.0f, 7));
    verts.push_back(pos + glm::vec4(0.3 * direction.x, 0, 0.3 * direction.y, 0));
    verts.push_back(nor);
    verts.push_back(color);
    verts.push_back(glm::vec4((uv.x + 1.0 / 16 * 3) / 16., uv.y / 16., 130.0f, 7));
    verts.push_back(pos + glm::vec4(0.3 * direction.x, 0.3, 0.3 * direction.y, 0));
    verts.push_back(nor);
    verts.push_back(color);
    verts.push_back(glm::vec4((uv.x + 1.0 / 16 * 3) / 16., (uv.y + 1.0 / 16 * 3) / 16., 130.0f, 8));
    verts.push_back(pos + glm::vec4(0, 0.3, 0, 0));
    verts.push_back(nor);
    verts.push_back(color);
    verts.push_back(glm::vec4(uv.x / 16., (uv.y + 1.0 / 16 * 3) / 16., 130.0f, 8));
}

void Snow::create()
{
    std::vector<glm::vec4> verts;
    std::vector<GLint> idx;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int index = i * 16 + j;
            createRect(verts, idx, m_pos[index], m_direction[index], m_velocity[index]);
        }
    }


    count1 = idx.size();

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx1();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx1);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generateVer1();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVer1);
    context->glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec4), verts.data(), GL_STATIC_DRAW);
}
