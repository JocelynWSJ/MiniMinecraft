#include "worldaxes.h"

void WorldAxes::create()
{

    GLuint idx[6] = {0, 1, 2, 3, 4, 5};
    glm::vec4 verts[24] = {
        glm::vec4(32,129,32,1),
        glm::vec4(0),
        glm::vec4(1,0,0,1),
        glm::vec4(0),
        glm::vec4(40,129,32,1),
        glm::vec4(0),
        glm::vec4(1,0,0,1),
        glm::vec4(0),
        glm::vec4(32,129,32,1),
        glm::vec4(0),
        glm::vec4(0,1,0,1),
        glm::vec4(0),
        glm::vec4(32,137,32,1),
        glm::vec4(0),
        glm::vec4(0,1,0,1),
        glm::vec4(0),
        glm::vec4(32,129,32,1),
        glm::vec4(0),
        glm::vec4(0,0,1,1),
        glm::vec4(0),
        glm::vec4(32,129,40,1),
        glm::vec4(0),
        glm::vec4(0,0,1,1),
        glm::vec4(0)
    };

    count0 = 6;

    generateIdx0();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx0);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generateVer0();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVer0);
    context->glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(glm::vec4), verts, GL_STATIC_DRAW);
}

GLenum WorldAxes::drawMode()
{
    return GL_LINES;
}
