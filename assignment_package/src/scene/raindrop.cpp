#include "raindrop.h"
#include <la.h>
#include <iostream>

float RainDrop::getOffset(int x, int z) const
{
    return m_offset[x * 16 + z];
}

void RainDrop::setOffset(int x, int z, float offset)
{
    m_offset[x * 16 + z] = offset;
}

float RainDrop::getHeight(int x, int z) const
{
    return m_height[x * 16 + z];
}

void RainDrop::setHeight(int x, int z, float height)
{
    m_height[x * 16 + z] = height + rand() / RAND_MAX;
}

bool RainDrop::getShouldBounced() const
{
    return shouldBounced;
}

void RainDrop::setShouldBounced(bool value)
{
    shouldBounced = value;
}

RainDrop::RainDrop(OpenGLContext* context, glm::vec4 pos,
                   glm::vec4 color)
    : Drawable(context),
      m_originPos(pos),
      m_color(color) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            m_pos.push_back(glm::vec4(m_originPos.x + i,
                                      m_originPos.y + (double)rand() / RAND_MAX * 100,
                                      m_originPos.z + j,
                                      1));
            m_direction.push_back(glm::vec2(((double)rand() / RAND_MAX - 0.5) * 2, ((double)rand()/ RAND_MAX - 0.5) * 2));
            m_offset.push_back(((double)rand() / RAND_MAX) * 4 + 10);
            if (i % 2 == 0 && j % 2 == 0) {
                m_height.push_back(129 + (double)rand() / RAND_MAX);
            }
            else {
                m_height.push_back(-1);
            }
        }
    }
}

glm::vec4 RainDrop::getOriginPos() const
{
    return m_originPos;
}

void RainDrop::setOriginPos(const glm::vec4 &pos)
{
    m_originPos = pos;
}

void RainDrop::createRect(std::vector<glm::vec4>& verts,
                          std::vector<GLint>& idx,
                          glm::vec4& pos,
                          glm::vec2& direction,
                          float offset, float height) {
    int count = verts.size() / 4;
    idx.push_back(count);
    idx.push_back(count + 1);
    idx.push_back(count + 2);
    idx.push_back(count);
    idx.push_back(count + 2);
    idx.push_back(count + 3);
    direction = glm::normalize(direction);

    glm::vec4 nor(-direction.y, 0, -direction.x, 0);
    verts.push_back(pos);
    verts.push_back(nor);
    verts.push_back(m_color);
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 2));
    verts.push_back(pos + glm::vec4(0.1 * direction.x, 0, 0.1 * direction.y, 0));
    verts.push_back(nor);
    verts.push_back(m_color);
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 2));
    verts.push_back(pos + glm::vec4(0.1 * direction.x, 1, 0.1 * direction.y, 0));
    verts.push_back(nor);
    verts.push_back(m_color);
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 3));
    verts.push_back(pos + glm::vec4(0, 1, 0, 0));
    verts.push_back(nor);
    verts.push_back(m_color);
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 3));

    glm::vec4 bpos(pos.x, height + 1, pos.z, 1);
    glm::vec4 bcolor(0.18, 0.34, 0.66, 0.8);

    if (height > 0 && int(pos.x) % 2 == 0 && int(pos.z) % 2 == 0) {
        for (int i = 0; i < 3; i++) {
            count = verts.size() / 4;
            idx.push_back(count);
            idx.push_back(count + 1);
            idx.push_back(count + 2);
            idx.push_back(count);
            idx.push_back(count + 2);
            idx.push_back(count + 3);

            verts.push_back(bpos);
            verts.push_back(nor);
            verts.push_back(bcolor);
            verts.push_back(glm::vec4(-1.0f, offset, height, 4));
            verts.push_back(bpos + glm::vec4(0.1 * direction.x, 0, 0.1 * direction.y, 0));
            verts.push_back(nor);
            verts.push_back(bcolor);
            verts.push_back(glm::vec4(-1.0f, offset, height, 4));
            verts.push_back(bpos + glm::vec4(0.1 * direction.x, 0.1, 0.1 * direction.y, 0));
            verts.push_back(nor);
            verts.push_back(bcolor);
            verts.push_back(glm::vec4(-1.0f, offset, height, 4));
            verts.push_back(bpos + glm::vec4(0, 0.1, 0, 0));
            verts.push_back(nor);
            verts.push_back(bcolor);
            verts.push_back(glm::vec4(-1.0f, offset, height, 4));

            if (i == 0) {
                bpos += glm::vec4(0.1 * direction.x, 0.1, 0.1 * direction.y, 0);
            }
            else {
                bpos -= glm::vec4(0.2 * direction.x, 0, 0.2 * direction.y, 0);
            }
            bcolor += glm::vec4(0.12, 0.12, 0.012, 0);
            if (int(pos.x + pos.z) % 6 == 0) {
                break;
            }
        }
    }
}

void RainDrop::create()
{
    std::vector<glm::vec4> verts;
    std::vector<GLint> idx;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int index = i * 16 + j;
            createRect(verts, idx, m_pos[index], m_direction[index], m_offset[index], m_height[index]);
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
