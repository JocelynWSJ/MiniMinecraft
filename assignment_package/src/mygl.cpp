#include "mygl.h"

#include <iostream>
#include <QApplication>
#include <QKeyEvent>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_worldAxes(mkU<WorldAxes>(this)),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)),
      mp_progSky(mkU<ShaderProgram>(this)), mp_geomQuad(mkU<Quad>(this)),
      mp_progLambVC(mkU<ShaderProgram>(this)), vao(0),
      mp_camera(mkU<Camera>()), mp_terrain(mkU<Terrain>(this)), mp_player(mkU<Player>(this)),
      /*mp_thirdperson(mkU<ThirdPerson>(this, glm::vec3(), glm::vec3())),*/ mp_lsystem(mkU<LSystem>(mp_terrain.get())),
      mp_npcsystem(mkU<NPCSystem>(this, mp_terrain.get())),
      mp_texture(mkU<Texture>(this)), mp_normalMap(mkU<Texture>(this)), m_time(0), timer(),
      currentTime(0), elapsedTime(0), lastPos(), flyLastFrame(0), jumpLastFrame(0)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    // set mouse position
    MoveMouseToCenter();
    lastPos = QCursor::pos();

    mp_player->camera = mp_camera.get();
//    mp_player->person = mp_thirdperson.get();
    currentTime = QDateTime::currentMSecsSinceEpoch();

    // initial 16 chunk creation has been done in terrain's constructor
    // initial 16 chunk update for L-system and assests
    mp_lsystem->update(Rect64(0, 0));
    for (int i = 0; i < 16; i++) {
        mp_terrain->placeAssets(Rect16(i / 4 * 16, i % 4 * 16));
    }
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_terrain->destroy();
    mp_npcsystem->destroy();
    //timer.stop();
}


void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    // glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);

    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    //glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    // Create the instance of Cube
    mp_worldAxes->create();
    mp_geomQuad->create();
    mp_terrain->create();
    mp_texture->create(":/assets/minecraft_textures_all.png");
    mp_texture->load(0);
    mp_normalMap->create(":/assets/minecraft_normals_all.png");
    mp_normalMap->load(1);

    // initial 16 chunk update for NPC system
    // do it here because it uses vbo
    for (int i = 0; i < 16; i++) {
        mp_npcsystem->birthNPC(Rect16(i / 4 * 16, i % 4 * 16));
    }

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");

    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Create Sky
    mp_progSky->create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");

    // Create and set up vertex color shader
    mp_progLambVC->create(":/glsl/lambertvc.vert.glsl", ":/glsl/lambertvc.frag.glsl");
  
    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    // vao.bind();
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    // This code sets the concatenated view and perspective projection matrices used for
    // our scene's camera view.

    *mp_camera = Camera(w, h, glm::vec3(22.f, 140.f, 22.f),
                       glm::vec3(33.f, 140.f, 33.f), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
    mp_progSky->setViewProjMatrix(glm::inverse(viewproj));
    // Sky
    mp_progSky->setDimensions(width(), height());
    mp_progSky->setEyePos(mp_camera->eye);
    mp_progLambVC->setViewProjMatrix(viewproj);

    printGLErrorLog();
}

glm::vec3 MyGL::getThirdPersonDir() {
    glm::vec3 dir;
    dir = -mp_camera->look;
    dir.y = 0;
    dir = glm::normalize(dir);
    dir *= 2.0;
    dir.y = 1.5;
    return dir;
}

// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    // FUNC1: Compute the time elapsed since the last update call.
    int64_t time = QDateTime::currentMSecsSinceEpoch();
    elapsedTime = time - currentTime;
    currentTime = time;

    // FUNC2: Based on the controller state, update the relevant attributes of the entity
    mp_player->velocity[1] = -3 * amount;  //enable gravity
    glm::vec3 camPos = mp_player->getPosition();
    if (mp_player->status[mp_player->getIdThirdPerson()]) {
        camPos -= getThirdPersonDir();
    }
    glm::vec3 bodyPos = camPos;
    bodyPos.y -= 1.3;
    if (mp_player->status[mp_player->getIdW()]) { //move forwards
        mp_player->velocity[2] = amount;
        if (mp_player->status[mp_player->getIdF()]) {
            mp_player->velocity[2] = 3 * amount;
        }
    }
    if (mp_player->status[mp_player->getIdS()]) { //move backwards
        mp_player->velocity[2] = -amount;
        if (mp_player->status[mp_player->getIdF()]) {
            mp_player->velocity[2] = -3 * amount;
        }
    } else {
        if (!mp_player->status[mp_player->getIdW()]) {
            mp_player->velocity[2] = 0;
        }
    }

    if (mp_player->status[mp_player->getIdD()]) { //move right
        mp_player->velocity[0] = amount;
        if (mp_player->status[mp_player->getIdF()]) {
            mp_player->velocity[0] = 3 * amount;
        }
    }
    if (mp_player->status[mp_player->getIdA()]) { //move left
        mp_player->velocity[0] = -amount;
        if (mp_player->status[mp_player->getIdF()]) {
            mp_player->velocity[0] = -3 * amount;
        }
    } else {
        if(!mp_player->status[mp_player->getIdD()]) {
            mp_player->velocity[0] = 0;
        }
    }

    if (mp_player->status[mp_player->getIdF()]) { //when flying
        if (mp_player->status[mp_player->getIdQ()]) { //move down
            mp_player->velocity[1] = -amount;
        }
        if (mp_player->status[mp_player->getIdE()]) { //move up
            mp_player->velocity[1] = amount;
        } else if(!mp_player->status[mp_player->getIdQ()]) {
            mp_player->velocity[1] = 0;
        }
        if (mp_player->status[mp_player->getIdRise()]) { //start flying, RISE
            // velocity constant decrease
            mp_player->velocity[1] = 2*amount - flyLastFrame * 0.01;
            flyLastFrame++;// set lasting time of rise
            if (mp_player->velocity[1] <= 0) {
                mp_player->status[mp_player->getIdRise()] = false;
                mp_player->velocity[1] = -amount;
                flyLastFrame = 0;
            }
        }
    }

    if (mp_player->status[mp_player->getIdSpacebar()] && !mp_player->status[mp_player->getIdF()]) { //jump or swim
        if (mp_player->status[mp_player->getIdSwim()]) { //if character is swimming
            mp_player->velocity[1]  = amount * 0.67f;
        } else { //if character is jumping on the ground
            mp_player->velocity[1] = 2*amount - jumpLastFrame * 0.01;
            jumpLastFrame++;
            if (mp_player->velocity[1] <= 0) {
                mp_player->status[mp_player->getIdSpacebar()] = false;
                mp_player->velocity[1] = 0;
                jumpLastFrame = 0;
            }
        }
    }

    // FUNC4: Prevent physics entities from colliding with other physics entities.
    // when the character is swimming, change its velocity to 2/3
    if (mp_player->status[mp_player->getIdSwim()]) {
        mp_player->velocity *= 0.67f;
    }
    // calculate the next position and its move trend
    glm::vec3 nextPos = bodyPos + (mp_player->velocity[0] * glm::vec3(mp_camera->right[0], 0, mp_camera->right[2])
            + mp_player->velocity[2] * glm::vec3(mp_camera->look[0], 0, mp_camera->look[2]) + glm::vec3(0, mp_player->velocity[1], 0));
    glm::vec3 moveTrend = nextPos - bodyPos;

    if (!mp_player->status[mp_player->getIdF()] && !mp_player->status[mp_player->getIdSpacebar()]) { // not flying
        if (VertCollisionDetect(bodyPos, moveTrend)) { //has contact the land, velocity[1] should be 0
            mp_player->velocity[1] = 0;
            // calculate new nextPos and MoveTrend after altering the velocity
            nextPos = bodyPos + mp_player->velocity[0] * glm::vec3(mp_camera->right[0], 0, mp_camera->right[2])
                    + mp_player->velocity[2] * glm::vec3(mp_camera->look[0], 0, mp_camera->look[2]);
            moveTrend = nextPos - bodyPos;
            if (!HerizCollisionDetect(bodyPos, moveTrend)) { // have no collision herizonal
                mp_player->updatePosition();
            }
        } else {
            if (!HerizCollisionDetect(bodyPos, moveTrend)) { // have no collision herizonal
                mp_player->updatePosition();
            }
        }
    } else {
        mp_player->updatePosition();
    }

    // update mouse move event
    QPoint cursorPos = QCursor::pos();
    mp_player->cursorChange = glm::vec2(cursorPos.x() - lastPos.x(),
                                        cursorPos.y() - lastPos.y());
    mp_player->updateRotAmongCursor();
    // reset mouse position to center
    MoveMouseToCenter();
    lastPos = QCursor::pos();

    Rect16 rect(0, 0);
    if (ThreadData::workerRunning == 0) {
        if (mp_terrain->checkBooarder((int)(camPos[0]), (int)(camPos[2]), rect)) {
            ThreadData::workerRunning = 1;
            Worker *worker = new Worker(mp_terrain.get(), mp_lsystem.get(), rect);
            QThreadPool::globalInstance()->start(worker);
        }
    } else if (ThreadData::workerRunning == 2) {
        rect = ThreadData::workerRect;
        if (mp_terrain->hasChunk(rect.xmid(), rect.zmid())) {
            Chunk* chunk = mp_terrain->getChunk(rect.xmid(), rect.zmid());
            chunk->updateSelf(&(ThreadData::info));
            chunk->updateNeighbor(&(ThreadData::linfo), &(ThreadData::rinfo),
                                  &(ThreadData::finfo), &(ThreadData::binfo));
            updateWeather(rect.xmid(), rect.zmid());
            mp_npcsystem->birthNPC(rect);
        }
        ThreadData::workerRunning = 0;
    }

    // update movement of npcs
    mp_npcsystem->update((float)elapsedTime);

    update();
}

void MyGL::updateWeather(int x, int z) {
    mp_terrain->moveToOrigin(x, z);
    if (mp_terrain->canRain(x + 8, z + 8)) {
        mp_terrain->m_rain.find(mp_terrain->hash(x, z))->second.destroy();
        mp_terrain->m_rain.find(mp_terrain->hash(x, z))->second.create();
    } else if (mp_terrain->canSnow(x + 8, z + 8)) {
        mp_terrain->m_snow.find(mp_terrain->hash(x, z))->second.destroy();
        mp_terrain->m_snow.find(mp_terrain->hash(x, z))->second.create();
    }
}

bool MyGL::HerizCollisionDetect(glm::vec3 pos, glm::vec3 movetrend) {
    int blockX, blockY, blockZ;
    blockY = (int)(floorf(pos[1]));
    blockX = (int)(floorf(pos[0] + movetrend[0]));
    blockZ = (int)(floorf(pos[2] + movetrend[2]));
    if (mp_terrain->getBlockAt(blockX, blockY, blockZ) != EMPTY) {
        if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == SNOW ||
            mp_terrain->getBlockAt(blockX, blockY, blockZ) == LEAF ||
            mp_terrain->getBlockAt(blockX, blockY, blockZ) == REDFLOWER ||
            mp_terrain->getBlockAt(blockX, blockY, blockZ) == CROSSGRASS ||
            mp_terrain->getBlockAt(blockX, blockY, blockZ) == MUSHROOM
                ) {
            return false;
        }
        if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == WATER || mp_terrain->getBlockAt(blockX, blockY, blockZ) == LAVA) {
            mp_player->status[mp_player->getIdSwim()] = true;
            return false;
        } else {
            mp_player->status[mp_player->getIdSwim()] = false;
        }
        return true;
    }
    mp_player->status[mp_player->getIdSwim()] = false;
    blockY += 1;
    if (mp_terrain->getBlockAt(blockX, blockY, blockZ) != EMPTY) {
        return true;
    }
    return false;
}

bool MyGL::VertCollisionDetect(glm::vec3 pos, glm::vec3 movetrend) {
    int blockX, blockY, blockZ;
    blockY = (int)(floorf(pos[1] + movetrend[1]));
    blockX = (int)(floorf(pos[0]));
    blockZ = (int)(floorf(pos[2]));

    if (movetrend[1] > 0) { //if the character is rising, test its head but not bottom block
        if (mp_player->status[mp_player->getIdSwim()]) { // if the character is swimming
            if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == EMPTY) {
                mp_player->status[mp_player->getIdSwim()] = false; //swimming stops.
            }
        }
        blockY += 2;
    }
    if (mp_terrain->getBlockAt(blockX, blockY, blockZ) != EMPTY) {
        if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == SNOW ||
                mp_terrain->getBlockAt(blockX, blockY, blockZ) == LEAF ||
                mp_terrain->getBlockAt(blockX, blockY, blockZ) == REDFLOWER ||
                mp_terrain->getBlockAt(blockX, blockY, blockZ) == CROSSGRASS ||
                mp_terrain->getBlockAt(blockX, blockY, blockZ) == MUSHROOM
                ) {
            return false;
        }
        // next block is water or lava, begin swimming
        if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == WATER || mp_terrain->getBlockAt(blockX, blockY, blockZ) == LAVA) { //begin swimming
            mp_player->status[mp_player->getIdSwim()] = true;
            return false;
        }
        return true;
    }
    return false;
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Render to our framebuffer rather than the viewport
    //glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    //glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progSky->setViewProjMatrix(glm::inverse(mp_camera->getViewProj()));
    mp_progLambVC->setViewProjMatrix(mp_camera->getViewProj());

    mp_progLambert->setTime(m_time);
    mp_progSky->setTime(m_time);
    mp_progSky->setEyePos(mp_camera->eye);
    mp_progLambVC->setTime(m_time);
    m_time++;

    GLDrawScene();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progFlat->draw(*mp_worldAxes);
    glEnable(GL_DEPTH_TEST);

    mp_texture->bind(0);
    mp_normalMap->bind(1);
}

void MyGL::GLDrawScene()
{
    glm::vec3 camPos = mp_player->getPosition();
    glm::vec3 direction = glm::normalize(mp_player->camera->look);
    int blockX, blockY, blockZ;
    blockX = (int)(floorf(camPos[0]));
    blockY = (int)(floorf(camPos[1]));
    blockZ = (int)(floorf(camPos[2]));
    Terrain* terrain = mp_terrain.get();
    int x = blockX;
    int z = blockZ;
    terrain->moveToOrigin(x, z);


    if (mp_terrain->canRain(x + 8, z + 8) && direction.y > 0.8) {
        mp_progSky->setBlendType(1);
    }
    else if (mp_terrain->canSnow(x + 8, z + 8) && direction.y > 0.8) {
        mp_progSky->setBlendType(2);
    }
    else if ((blockZ <= 256 && direction.z > 0.2) || (blockZ > 256 && direction.z < -0.2)) {
    //else if (direction.z > 0.4) {
        mp_progSky->setBlendType(3);
    }
    mp_progSky->draw(*mp_geomQuad);

    if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == LAVA) {
        mp_progLambert->setEnvironment(2);
        mp_progSky->setEnvironment(2);
        mp_progLambVC->setEnvironment(2);
    } else if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == WATER) {
        mp_progLambert->setEnvironment(1);
        mp_progSky->setEnvironment(1);
        mp_progLambVC->setEnvironment(1);
    } else {
        mp_progLambert->setEnvironment(0);
        mp_progSky->setEnvironment(0);
        mp_progLambVC->setEnvironment(0);
    }

    if (mp_terrain->canRain(x + 8, z + 8) && direction.y > 0.8) {
        mp_progLambert->setBlendType(1);
    } else if (mp_terrain->canSnow(x + 8, z + 8) && direction.y > 0.8) {
        mp_progLambert->setBlendType(2);
    }

    for (auto it = terrain->m_chunks.begin(); it != terrain->m_chunks.end(); it++) {
        mp_progLambert->setModelMatrix(glm::mat4());
        mp_progLambert->draw(it->second, 0);
    }
    for (unsigned int i = 0; i < mp_npcsystem->npcs.size(); i++) {
        NPC *npc = mp_npcsystem->npcs[i].get();
        for (unsigned int j = 0; j < npc->size(); j++) {
            mp_progLambVC->setModelMatrix(npc->partTrans(j));
            mp_progLambVC->draw(*(npc->partAt(j)), 0);
        }
    }
    if (mp_terrain->getBlockAt(blockX, blockY, blockZ) == LAVA || mp_terrain->getBlockAt(blockX, blockY, blockZ) == WATER) {
            glDisable(GL_CULL_FACE);
            for (auto it = terrain->m_chunks.begin(); it != terrain->m_chunks.end(); it++) {
                mp_progLambert->setModelMatrix(glm::mat4());
                mp_progLambert->draw(it->second, 1);
            }
            glEnable(GL_CULL_FACE);
        } else {
            for (auto it = terrain->m_chunks.begin(); it != terrain->m_chunks.end(); it++) {
                mp_progLambert->setModelMatrix(glm::mat4());
                mp_progLambert->draw(it->second, 1);
            }
        }
    for (auto it = terrain->m_rain.begin(); it != terrain->m_rain.end(); it++) {
        mp_progLambert->setModelMatrix(glm::mat4());
        mp_progLambert->draw(it->second, 1);
    }
    for (auto it = terrain->m_snow.begin(); it != terrain->m_snow.end(); it++) {
        mp_progLambert->setModelMatrix(glm::mat4());
        mp_progLambert->draw(it->second, 1);
    }
    glDisable(GL_CULL_FACE);
    for (auto it = terrain->m_lightening.begin(); it != terrain->m_lightening.end(); it++) {
        mp_progLambert->setModelMatrix(glm::mat4());
        mp_progLambert->draw(it->second, 1);
    }
    glEnable(GL_CULL_FACE);

    mp_progSky->setBlendType(0);
    mp_progLambert->setBlendType(0);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_R) {
        *mp_camera = Camera(this->width(), this->height());
        mp_player->camera = mp_camera.get();
    } else if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_T) {
        if (!mp_player->status[mp_player->getIdThirdPerson()]) {
            mp_camera->eye += getThirdPersonDir();
        } else {
            mp_camera->eye -= getThirdPersonDir();
        }
    }
    mp_player->KeyEventListener(e);
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) { //move forward
        mp_player->status[mp_player->getIdW()] = false;
    } else if (e->key() == Qt::Key_S) { //move backwards
        mp_player->status[mp_player->getIdS()] = false;
    } else if (e->key() == Qt::Key_D) { //move right
        mp_player->status[mp_player->getIdD()] = false;
    } else if (e->key() == Qt::Key_A) { //move left
        mp_player->status[mp_player->getIdA()] = false;
    } else if (e->key() == Qt::Key_Q) { //move right
        mp_player->status[mp_player->getIdQ()] = false;
    } else if (e->key() == Qt::Key_E) { //move left
        mp_player->status[mp_player->getIdE()] = false;
    } else if (e->key() == Qt::Key_Space) { //move left
        if (mp_player->status[mp_player->getIdSwim()]) {
            mp_player->status[mp_player->getIdSpacebar()] = false;
        }
    }
}

void MyGL::mousePressEvent(QMouseEvent *m) {
    if (m->button() == Qt::LeftButton) {
        mp_player->status[mp_player->getIdLeft()] = true;
        mp_terrain->playerClick(mp_camera->eye, mp_camera->look, false);
    } else if (m->button() == Qt::RightButton) {
        mp_player->status[mp_player->getIdRight()] = true;
        mp_terrain->playerClick(mp_camera->eye, mp_camera->look, true);
    }
}

void MyGL::mouseReleaseEvent(QMouseEvent *m) {
    if (m->button() == Qt::LeftButton) {
        mp_player->status[mp_player->getIdLeft()] = false;
    } else if (m->button() == Qt::RightButton) {
        mp_player->status[mp_player->getIdRight()] = false;
    }
}
