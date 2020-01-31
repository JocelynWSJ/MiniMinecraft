#include "quad.h"
#include "drawable.h"
#include <la.h>

Quad::Quad(OpenGLContext *context) : Drawable(context)
{}

void Quad::create()
{
    GLuint index[6]{0, 1, 2, 0, 2, 3};
    glm::vec4 vert_pos[4] {glm::vec4(-1.f, -1.f, 0.999999f, 1.f),
                           glm::vec4(1.f, -1.f, 0.999999f, 1.f),
                           glm::vec4(1.f, 1.f, 0.999999f, 1.f),
                           glm::vec4(-1.f, 1.f, 0.999999f, 1.f)};
    glm::vec2 vert_UV[4] {glm::vec2(0.f, 0.f),
                          glm::vec2(1.f, 0.f),
                          glm::vec2(1.f, 1.f),
                          glm::vec2(0.f, 1.f)};

    std::vector<float> pos;
    std::vector<GLuint> idx;
    for (int i = 0; i < 6; i++) {
        idx.push_back(index[i]);
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            pos.push_back(vert_pos[i][j]);
        }
        for (int j = 0; j < 8; j++) {
            pos.push_back(0);
        }
        for (int j = 0; j < 2; j++) {
            pos.push_back(vert_UV[i][j]);
        }
        for (int j = 0; j < 2; j++) {
            pos.push_back(0);
        }
    }

    count0 = 6;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx0();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx0);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generateVer0();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVer0);
    context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), pos.data(), GL_STATIC_DRAW);

}
