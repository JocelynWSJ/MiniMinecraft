#ifndef PLAYER_H
#define PLAYER_H
#pragma once

#include <la.h>
#include <QKeyEvent>
#include <QMouseEvent>
#include "camera.h"
#include "scene/npcsystem.h"

//class ThirdPerson : public NPC{
//public:
//    ThirdPerson(OpenGLContext *context, const glm::vec3 &pos, const glm::vec3 &rot):
//        NPC(context, pos, rot,
//            Hexahedron(-0.5f, 0.5f, 0.f, 2.f, -0.5f, 0.5f, glm::vec3()))
//    { /*birth(); create();*/}
//    virtual ~ThirdPerson() {}
//    void update(float deltaTime, float totalTime) override;
//private:
//    void birth() override;
//    void idle(float time);
//};

class Player
{
public:
    Player(OpenGLContext *context);

    OpenGLContext *context;
//    ThirdPerson* person;

    Camera* camera; //A pointer to a Camera
    //glm::vec3 position; //A position in 3D space
    glm::vec3 velocity; //A velocity in 3D space

    glm::vec3 getPosition();
    //A set of variables to track the relevant inputs from the mouse and keyboard.
    //Including W, A, S, D, and Spacebar keys
    //Including the change in the cursor's X and Y coordinates
    //Including the state of its left and right mouse buttons.
    ///Use getId() function to get corresponding index
    bool status[13];

    //track the change in the cursor's X and Y coordinates
    //first element represents X change, second represents Y change
    glm::vec2 cursorChange;
    void updateRotAmongCursor();
    void updatePosition();

    //A function that takes in a QKeyEvent from MyGL's key press
    //and key release event functions, and updates the relevant
    //member variables based on the event.
    void KeyEventListener(QKeyEvent *e);

    void playerJump();

    int getIdLeft();
    int getIdRight();
    int getIdW();
    int getIdA();
    int getIdS();
    int getIdD();
    int getIdSpacebar();
    int getIdF();
    int getIdRise();
    int getIdSwim();
    int getIdQ();
    int getIdE();
    int getIdThirdPerson();





};

#endif // PLAYER_H
