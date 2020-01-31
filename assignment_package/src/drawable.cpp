#include "drawable.h"
#include <la.h>

Drawable::Drawable(OpenGLContext* context)
    : bufIdx0(), bufVer0(),
      idxBound0(false), verBound0(false),
      bufIdx1(), bufVer1(),
      idxBound1(false), verBound1(false),
      context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdx0);
    context->glDeleteBuffers(1, &bufVer0);
    context->glDeleteBuffers(1, &bufIdx1);
    context->glDeleteBuffers(1, &bufVer1);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount0()
{
    return count0;
}

int Drawable::elemCount1()
{
    return count1;
}

void Drawable::generateIdx0()
{
    idxBound0 = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdx0);
}

void Drawable::generateVer0()
{
    verBound0 = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufVer0);
}

bool Drawable::bindIdx0()
{
    if(idxBound0) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx0);
    }
    return idxBound0;
}

bool Drawable::bindVer0()
{
    if(verBound0){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufVer0);
    }
    return verBound0;
}

void Drawable::generateIdx1()
{
    idxBound1 = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdx1);
}

void Drawable::generateVer1()
{
    verBound1 = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufVer1);
}

bool Drawable::bindIdx1()
{
    if(idxBound1) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx1);
    }
    return idxBound1;
}

bool Drawable::bindVer1()
{
    if(verBound1){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufVer1);
    }
    return verBound1;
}
