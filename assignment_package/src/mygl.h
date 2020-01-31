#ifndef MYGL_H
#define MYGL_H

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QDateTime>
#include <QThreadPool>

#include "shaderprogram.h"
#include "scene/npcsystem.h"
#include "scene/worldaxes.h"
#include "scene/quad.h"
#include "camera.h"
#include "openglcontext.h"
#include "player.h"
#include "texture.h"
#include "utils.h"
#include "worker.h"

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    // The screen-space quadrangle used to draw
    // the scene with the post-process shaders.
    uPtr<WorldAxes> mp_worldAxes;       // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    uPtr<ShaderProgram> mp_progLambert; // A shader program that uses lambertian reflection
    uPtr<ShaderProgram> mp_progFlat;    // A shader program that uses "flat" reflection (no shadowing at all)
    uPtr<ShaderProgram> mp_progSky;     // A screen-space shader for creating the sky background
    uPtr<Quad> mp_geomQuad;
    uPtr<ShaderProgram> mp_progLambVC;  // A shader program that uses vertex color

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.
    // A collection of handles to the five frame buffers we've given
    // ourselves to perform render passes. The 0th frame buffer is always
    // written to by the render pass that uses the currently bound surface shader.
    GLuint m_renderedTexture;
    // A collection of handles to the depth buffers used by our frame buffers.
    // m_frameBuffers[i] writes to m_depthRenderBuffers[i].

    uPtr<Camera> mp_camera;
    uPtr<Terrain> mp_terrain;
    uPtr<Player> mp_player;
//    uPtr<ThirdPerson> mp_thirdperson;
    uPtr<LSystem> mp_lsystem;
    uPtr<NPCSystem> mp_npcsystem;

    uPtr<Texture> mp_texture;
    uPtr<Texture> mp_normalMap;
    // A variable used to keep track of the time elapsed.
    // It increments by 1 at the end of each call of paintGL().
    // In paintGL, it is passed to the currently bound surface and post-process shaders,
    // if they have a uniform variable for time.
    int m_time;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;
    int64_t currentTime;
    int64_t elapsedTime;
    int64_t jumpStartTime;

    QPoint lastPos;
    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    bool HerizCollisionDetect(glm::vec3 pos, glm::vec3 moveTrend); //if character is at position pos, return true if collision happens;
    bool VertCollisionDetect(glm::vec3 pos, glm::vec3 movetrend);
    float amount = 0.1; //velocity
    int flyLastFrame;
    int jumpLastFrame;

    void createRenderBuffers();
    glm::vec3 getThirdPersonDir();

    void updateWeather(int x, int z);

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void GLDrawScene();

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *m);
    void mouseReleaseEvent(QMouseEvent *m);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
