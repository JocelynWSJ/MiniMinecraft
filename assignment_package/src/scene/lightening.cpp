#include "lightening.h"

#include <iostream>

// add face for a block
void Lightening::addFace(glm::vec4 pos0,
                         glm::vec4 pos1,
                         glm::vec4 nor,
                         std::vector<glm::vec4>& verts,
                         std::vector<GLuint>& idx) const {

    int count = verts.size() / 4;
    idx.push_back(count);
    idx.push_back(count + 1);
    idx.push_back(count + 2);
    idx.push_back(count);
    idx.push_back(count + 2);
    idx.push_back(count + 3);

    verts.push_back(pos0);
    verts.push_back(nor);
    verts.push_back(glm::vec4(1, 1, 1, 0.3));
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 5));
    verts.push_back(pos1);
    verts.push_back(nor);
    verts.push_back(glm::vec4(1, 1, 1, 0.3));
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 5));
    verts.push_back(glm::vec4(pos1.x, 255, pos1.z, 1));
    verts.push_back(nor);
    verts.push_back(glm::vec4(1, 1, 1, 0.3));
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 6));
    verts.push_back(glm::vec4(pos0.x, 255, pos0.z, 1));
    verts.push_back(nor);
    verts.push_back(glm::vec4(1, 1, 1, 0.3));
    verts.push_back(glm::vec4(-1.0f, -1.0f, 130.0f, 6));
}


void Lightening::create()
{
    std::vector<glm::vec4> verts;
    std::vector<GLuint> idx;

    glm::vec4 pos = m_originPos + glm::vec4(0.6, 0, 0.6, 0);
    for (int i = 0; i < 4; i++) {
        float length = 0.2 + 0.4 * i;
        addFace(pos, pos + glm::vec4(0, 0, length, 0), glm::vec4(-1, 0, 0, 0), verts, idx);
        addFace(pos + glm::vec4(0, 0, length, 0), pos + glm::vec4(length, 0, length, 0), glm::vec4(0, 0, 1, 0), verts, idx);
        addFace(pos + glm::vec4(length, 0, length, 0), pos + glm::vec4(length, 0, 0, 0), glm::vec4(1, 0, 0, 0), verts, idx);
        addFace(pos + glm::vec4(length, 0, 0, 0), pos, glm::vec4(0, 0, -1, 0), verts, idx);
        pos -= glm::vec4(0.2, 0, 0.2, 0);
    }


    //addFace(m_originPos, m_originPos + glm::vec4(0, 0, 1.4, 0), glm::vec4(-1, 0, 0, 0), verts, idx);
    //addFace(m_originPos + glm::vec4(0, 0, 1.4, 0), m_originPos + glm::vec4(1.4, 0, 1.4, 0), glm::vec4(0, 0, 1, 0), verts, idx);
    //addFace(m_originPos + glm::vec4(1.4, 0, 1.4, 0), m_originPos + glm::vec4(0, 0, 1.4, 0), glm::vec4(1, 0, 0, 0), verts, idx);
    //addFace(m_originPos + glm::vec4(1.4, 0, 0, 0), m_originPos, glm::vec4(0, 0, -1, 0), verts, idx);


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
