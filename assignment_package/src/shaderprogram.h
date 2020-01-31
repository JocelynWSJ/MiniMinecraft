#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <openglcontext.h>
#include <la.h>
#include <glm/glm.hpp>

#include "drawable.h"


class ShaderProgram
{
public:
    GLuint vertShader; // A handle for the vertex shader stored in this shader program
    GLuint fragShader; // A handle for the fragment shader stored in this shader program
    GLuint prog;       // A handle for the linked shader program stored in this class

    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrNor; // A handle for the "in" vec4 representing vertex normal in the vertex shader
    int attrCol; // A handle for the "in" vec4 representing vertex color in the vertex shader
    int attrUV; // A handle for the "in" vec2 representing UV in the vertex shader
    int attrCosine; // A handle for the "in" float representing cosine in the vertex shader
    int attrAnimated; // A handle for the "in" int representing block type in the vertex shader


    int unifModel; // A handle for the "uniform" mat4 representing model matrix in the vertex shader
    int unifModelInvTr; // A handle for the "uniform" mat4 representing inverse transpose of the model matrix in the vertex shader
    int unifViewProj; // A handle for the "uniform" mat4 representing combined projection and view matrices in the vertex shader
    int unifSampler2D; // A handle for the "uniform" vec4 representing texture
    int unifNormalMap; // A handle for the "uniform" vec4 representing texture
    int unifTime; // A handle for the "uniform" vec4 representing time
    int unifBlendType; // A handle for the "uniform" vec4 representing blend type
    int unifEnvironm; // A handle for the "uniform" int representing environment(lava, water)

    // Sky
    int unifDimensions;
    int unifEye;

public:
    ShaderProgram(OpenGLContext* context);
    // Sets up the requisite GL data and shaders from the given .glsl files
    void create(const char *vertfile, const char *fragfile);
    // Tells our OpenGL context to use this shader to draw things
    void useMe();
    // Pass the given model matrix to this shader on the GPU
    void setModelMatrix(const glm::mat4 &model);
    // Pass the given Projection * View matrix to this shader on the GPU
    void setViewProjMatrix(const glm::mat4 &vp);
    // Pass the given time to this shader on the GPU
    void setTime(int t);
    void setBlendType(int t);
    // Pass the dimension of screen to Sky shader
    void setDimensions(int width, int height);
    // Pass the camera's position to shader
    void setEyePos(const glm::vec3&);
    // Pass the environment to shader
    void setEnvironment(int e);
    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d, int bufferIdx = 0,int textureSlot = 0);
    // Utility function used in create()
    char* textFileRead(const char*);
    // Utility function that prints any shader compilation errors to the console
    void printShaderInfoLog(int shader);
    // Utility function that prints any shader linking errors to the console
    void printLinkInfoLog(int prog);

    QString qTextFileRead(const char*);

private:
    OpenGLContext* context;   // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                            // we need to pass our OpenGL context to the Drawable in order to call GL functions
                            // from within this class.
};


#endif // SHADERPROGRAM_H
