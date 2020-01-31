#include "player.h"
#include <iostream>


//void ThirdPerson::update(float deltaTime, float totalTime) {
//    idle(totalTime);
//}

//void ThirdPerson::birth() {
//    glm::vec3 white(1.f, 1.f, 1.f);
//    glm::vec3 black(0.f, 0.f, 0.f);
//    BodyPart head(m_context, glm::vec3(0.0, 2.0, 0.0));
//    float x[6] = {-0.5f, -0.25f, -0.125f, 0.125f, 0.25f, 0.5f};
//    float y[5] = {-1.2f, -1.f, -0.6f, -0.35f, 0.f};
//    float z[5] = {-0.5f, -0.4f, 0.f, 0.4f, 0.5f};
//    head.add(Hexahedron(x[0], x[5], y[1], y[4], z[0], z[2], white));
//    head.add(Hexahedron(x[1], x[4], y[2], y[3], z[1], z[3], black));
//    head.add(Hexahedron(x[0], x[1], y[0], y[4], z[0], z[4], white));
//    head.add(Hexahedron(x[2], x[3], y[0], y[4], z[0], z[4], white));
//    head.add(Hexahedron(x[4], x[5], y[0], y[4], z[0], z[4], white));
//    head.add(Hexahedron(x[0], x[5], y[1], y[2], z[0], z[4], white));
//    head.add(Hexahedron(x[0], x[5], y[3], y[4], z[0], z[4], white));
//    m_parts.push_back(head);
//}

//void ThirdPerson::idle(float time) {
//    if (m_parts.size() < 1) {
//        return;
//    }
//    m_parts[0].m_scale[1] = 1.f + 0.3f * sinf(time / 300.f);
//}
//*************************************************************************
Player::Player(OpenGLContext *context)
    : context(context), camera(nullptr), /*person(nullptr),*/ velocity(glm::vec3(0, 1.5, 0))
{
    for (int i = 0; i < 13; i++) {
        status[i] = false;
    }
    //status[getIdLand()] = true;
}

glm::vec3 Player::getPosition() {
    return camera->eye;
}

void Player::KeyEventListener(QKeyEvent *e) {
    if(e->modifiers() & Qt::ShiftModifier){
        velocity *= 2.f;
    }

    float amount = 0.15;
    if (e->key() == Qt::Key_Right) {
        camera->RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        camera->RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        camera->RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        camera->RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        camera->fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        camera->fovy -= amount;
    } else if (e->key() == Qt::Key_W) { //move forwards
        status[getIdW()] = true;
        velocity[0] = amount;
    } else if (e->key() == Qt::Key_S) { //move backwards
        status[getIdS()] = true;
        velocity[0] = -amount;
    } else if (e->key() == Qt::Key_D) { //move right
        status[getIdD()] = true;
        velocity[2] = amount;
    } else if (e->key() == Qt::Key_A) { //move left
        status[getIdA()] = true;
        velocity[2] = -amount;
    } else if (e->key() == Qt::Key_Q) {
        if (status[getIdF()]) {
            status[getIdQ()] = true;
        }
    } else if (e->key() == Qt::Key_E) {
        if (status[getIdF()]) {
            status[getIdE()] = true;
        }
    } else if (e->key() == Qt::Key_F) {
        if (!status[getIdF()]) {
            status[getIdRise()] = true;
            status[getIdF()] = true;
        } else {
            status[getIdRise()] = false;
            status[getIdF()] = false;
        }

    } else if (e->key() == Qt::Key_Space) {
        status[getIdSpacebar()] = true;
    } else if (e->key() == Qt::Key_T) {
        if (!status[getIdThirdPerson()]) {
            status[getIdThirdPerson()] = true;
        } else {
            status[getIdThirdPerson()] = false;
        }
    }
    camera->RecomputeAttributes();
}

void Player::updateRotAmongCursor() {
    //When the cursor's X position changes, the camera should rotate about the world up vector, i.e. the Y axis.
    if (cursorChange[0] != 0) {
        camera->RotateAboutWorldUp(-float(cursorChange[0])/5.0);
    }

    //When the cursor's Y position changes, the camera should rotate about its local right vector
    if (cursorChange[1] != 0) {
        camera->RotateAboutRight(-float(cursorChange[1])/8.0);
    }
}

void Player::updatePosition() {
    camera->TranslateAlongLook(velocity[2]);
    camera->TranslateAlongUp(velocity[1]);
    camera->TranslateAlongRight(velocity[0]);
}

void Player::playerJump() {
    camera->TranslateAlongUp(velocity[1]);
}

int Player::getIdLeft() {
    return 0;
}

int Player::getIdRight() {
    return 1;
}

int Player::getIdW() {
    return 2;
}

int Player::getIdA() {
    return 3;
}

int Player::getIdS() {
    return 4;
}

int Player::getIdD() {
    return 5;
}

int Player::getIdSpacebar() {
    return 6;
}

int Player::getIdF() {
    return 7;
}

int Player::getIdRise() {
    return 8;
}

int Player::getIdSwim() {
    return 9;
}

int Player::getIdQ() {
    return 10;
}

int Player::getIdE() {
    return 11;
}
int Player::getIdThirdPerson() {
    return 12;
}
