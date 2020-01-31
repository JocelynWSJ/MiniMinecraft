#pragma once

#include <openglcontext.h>
#include <la.h>

//This defines a class which can be rendered by our shader program.
//Make any geometry a subclass of ShaderProgram::Drawable in order to render it with the ShaderProgram class.
class Drawable
{
protected:
    int count0;     // The number of indices stored in bufIdx.
    GLuint bufIdx0; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint bufVer0; // A Vertex Buffer Object that we will use to store mesh vertices (vec4s)

    bool idxBound0; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool verBound0;

    int count1;     // The number of indices stored in bufIdx.
    GLuint bufIdx1; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint bufVer1; // A Vertex Buffer Object that we will use to store mesh vertices (vec4s)

    bool idxBound1; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool verBound1;

    OpenGLContext* context; // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                          // we need to pass our OpenGL context to the Drawable in order to call GL functions
                          // from within this class.


public:
    Drawable(OpenGLContext* context);
    virtual ~Drawable();

    virtual void create() = 0; // To be implemented by subclasses. Populates the VBOs of the Drawable.
    void destroy(); // Frees the VBOs of the Drawable.

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCount0();
    int elemCount1();

    // Call these functions when you want to call glGenBuffers on the buffers stored in the Drawable
    // These will properly set the values of idxBound etc. which need to be checked in ShaderProgram::draw()
    void generateIdx0();
    void generateVer0();

    bool bindIdx0();
    bool bindVer0();

    void generateIdx1();
    void generateVer1();

    bool bindIdx1();
    bool bindVer1();
};
