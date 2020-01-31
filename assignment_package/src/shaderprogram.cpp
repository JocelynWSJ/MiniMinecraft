#include "shaderprogram.h"
#include <QFile>
#include <QStringBuilder>
#include <QTextStream>
#include <QDebug>


ShaderProgram::ShaderProgram(OpenGLContext *context)
    : vertShader(), fragShader(), prog(),
      attrPos(-1), attrNor(-1), attrCol(-1),
      attrUV(-1), attrCosine(-1), attrAnimated(-1),
      unifModel(-1), unifModelInvTr(-1), unifViewProj(-1),
      unifSampler2D(-1), unifNormalMap(-1), unifTime(-1), unifBlendType(-1),
      unifEnvironm(-1), unifDimensions(-1), unifEye(-1),
      context(context)
{}

void ShaderProgram::create(const char *vertfile, const char *fragfile)
{
    // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
    vertShader = context->glCreateShader(GL_VERTEX_SHADER);
    fragShader = context->glCreateShader(GL_FRAGMENT_SHADER);
    prog = context->glCreateProgram();
    // Get the body of text stored in our two .glsl files
    QString qVertSource = qTextFileRead(vertfile);
    QString qFragSource = qTextFileRead(fragfile);

    char* vertSource = new char[qVertSource.size()+1];
    strcpy(vertSource, qVertSource.toStdString().c_str());
    char* fragSource = new char[qFragSource.size()+1];
    strcpy(fragSource, qFragSource.toStdString().c_str());


    // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
    context->glShaderSource(vertShader, 1, &vertSource, 0);
    context->glShaderSource(fragShader, 1, &fragSource, 0);
    // Tell OpenGL to compile the shader text stored above
    context->glCompileShader(vertShader);
    context->glCompileShader(fragShader);
    // Check if everything compiled OK
    GLint compiled;
    context->glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(vertShader);
    }
    context->glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(fragShader);
    }

    // Tell prog that it manages these particular vertex and fragment shaders
    context->glAttachShader(prog, vertShader);
    context->glAttachShader(prog, fragShader);
    context->glLinkProgram(prog);

    // Check for linking success
    GLint linked;
    context->glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        printLinkInfoLog(prog);
    }

    // Get the handles to the variables stored in our shaders
    // See shaderprogram.h for more information about these variables

    attrPos = context->glGetAttribLocation(prog, "vs_Pos");
    attrNor = context->glGetAttribLocation(prog, "vs_Nor");
    attrCol = context->glGetAttribLocation(prog, "vs_Col");
    attrUV = context->glGetAttribLocation(prog, "vs_UV");
    attrCosine = context->glGetAttribLocation(prog, "vs_Cosine");
    attrAnimated = context->glGetAttribLocation(prog, "vs_Animated");

    unifModel      = context->glGetUniformLocation(prog, "u_Model");
    unifModelInvTr = context->glGetUniformLocation(prog, "u_ModelInvTr");
    unifViewProj   = context->glGetUniformLocation(prog, "u_ViewProj");
    unifSampler2D  = context->glGetUniformLocation(prog, "u_Texture");
    unifNormalMap  = context->glGetUniformLocation(prog, "u_NormalMap");
    unifTime       = context->glGetUniformLocation(prog, "u_Time");
    unifEnvironm   = context->glGetUniformLocation(prog, "u_Envir");
    unifBlendType  = context->glGetUniformLocation(prog, "u_BlendType");
    // Sky demo
    unifDimensions = context->glGetUniformLocation(prog, "u_Dimensions");
    unifEye = context->glGetUniformLocation(prog, "u_Eye");

    delete[] vertSource;
    delete[] fragSource;

}

void ShaderProgram::useMe()
{
    context->glUseProgram(prog);
}

void ShaderProgram::setModelMatrix(const glm::mat4 &model)
{
    useMe();

    if (unifModel != -1) {
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModel,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &model[0][0]);
    }

    if (unifModelInvTr != -1) {
        glm::mat4 modelinvtr = glm::inverse(glm::transpose(model));
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModelInvTr,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &modelinvtr[0][0]);
    }
}

void ShaderProgram::setViewProjMatrix(const glm::mat4 &vp)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifViewProj != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniformMatrix4fv(unifViewProj,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                       &vp[0][0]);
    }
}

void ShaderProgram::setDimensions(int width, int height) {
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifDimensions != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniform2i(unifDimensions, width, height);
    }
}

void ShaderProgram::setEyePos(const glm::vec3 &pos) {
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifEye != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniform3f(unifEye, pos.x, pos.y, pos.z);
    }
}

void ShaderProgram::setTime(int t) {
    useMe();
    if(unifTime != -1)
    {
        context->glUniform1i(unifTime, t);
    }
}

void ShaderProgram::setEnvironment(int t) {
    useMe();
    if(unifEnvironm != -1)
    {
        context->glUniform1i(unifEnvironm, t);
    }
}

void ShaderProgram::setBlendType(int t) {
    useMe();
    if(unifBlendType != -1)
    {
        context->glUniform1i(unifBlendType, t);
    }
}

//This function, as its name implies, uses the passed in GL widget
void ShaderProgram::draw(Drawable &d, int bufferIdx, int)
{
    useMe();

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

        // Remember, by calling bindPos(), we call
        // glBindBuffer on the Drawable's VBO for vertex position,
        // meaning that glVertexAttribPointer associates vs_Pos
        // (referred to by attrPos) with that VBO

    if (unifSampler2D != -1)
    {
        context->glUniform1i(unifSampler2D, 0);
    }

    if (unifNormalMap != -1)
    {
        context->glUniform1i(unifNormalMap, 1);
    }

    // Draw Opaque
    if (bufferIdx == 0) {
        d.bindVer0();

        if (attrPos != -1) {
            context->glEnableVertexAttribArray(attrPos);
            context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)0);
        }

        if (attrNor != -1) {
            context->glEnableVertexAttribArray(attrNor);
            context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(4 * sizeof(float)));
        }

        if (attrCol != -1) {
            context->glEnableVertexAttribArray(attrCol);
            context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(8 * sizeof(float)));
        }

        if (attrUV != -1) {
            context->glEnableVertexAttribArray(attrUV);
            context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(12 * sizeof(float)));
        }

        if (attrCosine != -1) {
            context->glEnableVertexAttribArray(attrCosine);
            context->glVertexAttribPointer(attrCosine, 1, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(14 * sizeof(float)));
        }
        if (attrAnimated != -1) {
            context->glEnableVertexAttribArray(attrAnimated);
            context->glVertexAttribPointer(attrAnimated, 1, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(15 * sizeof(float)));
        }
        // Bind the index buffer and then draw shapes from it.
        // This invokes the shader program, which accesses the vertex buffers.
        d.bindIdx0();
        context->glDrawElements(d.drawMode(), d.elemCount0(), GL_UNSIGNED_INT, 0);

        if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
        if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
        if (attrCol != -1) context->glDisableVertexAttribArray(attrCol);
        if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);
        if (attrCosine != -1) context->glDisableVertexAttribArray(attrCosine);
    }
    else {
        // Transparency
        d.bindVer1();

        if (attrPos != -1) {
            context->glEnableVertexAttribArray(attrPos);
            context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)0);
        }

        if (attrNor != -1) {
            context->glEnableVertexAttribArray(attrNor);
            context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(4 * sizeof(float)));
        }

        if (attrCol != -1) {
            context->glEnableVertexAttribArray(attrCol);
            context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(8 * sizeof(float)));
        }

        if (attrUV != -1) {
            context->glEnableVertexAttribArray(attrUV);
            context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(12 * sizeof(float)));
        }

        if (attrCosine != -1) {
            context->glEnableVertexAttribArray(attrCosine);
            context->glVertexAttribPointer(attrCosine, 1, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(14 * sizeof(float)));
        }
        if (attrAnimated != -1) {
            context->glEnableVertexAttribArray(attrAnimated);
            context->glVertexAttribPointer(attrAnimated, 1, GL_FLOAT, false,
                                           16 * sizeof(float), (void*)(15 * sizeof(float)));
        }
        // Bind the index buffer and then draw shapes from it.
        // This invokes the shader program, which accesses the vertex buffers.
        d.bindIdx1();
        context->glDrawElements(d.drawMode(), d.elemCount1(), GL_UNSIGNED_INT, 0);

        if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
        if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
        if (attrCol != -1) context->glDisableVertexAttribArray(attrCol);
        if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);
        if (attrCosine != -1) context->glDisableVertexAttribArray(attrCosine);
    }
    context->printGLErrorLog();
}

char* ShaderProgram::textFileRead(const char* fileName) {
    char* text;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);

            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        }
    }
    return text;
}

QString ShaderProgram::qTextFileRead(const char *fileName)
{
    QString text;
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        text.append('\0');
    }
    return text;
}

void ShaderProgram::printShaderInfoLog(int shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
        qDebug() << "ShaderInfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}

void ShaderProgram::printLinkInfoLog(int prog)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog);
        qDebug() << "LinkInfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }
}
