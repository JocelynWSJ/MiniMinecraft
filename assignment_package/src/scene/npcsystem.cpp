#include "npcsystem.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "utils.h"
#include <iostream>

void Hexahedron::populate(std::vector<GLuint> &idx, std::vector<float> &info) const {
    //    2     3
    // 6     7
    //                y
    //    0     1     o x
    // 4     5      z
    glm::vec3 ver[24] = {
        pos[0], pos[1], pos[5], pos[4], pos[2], pos[6], pos[7], pos[3],
        pos[0], pos[4], pos[6], pos[2], pos[1], pos[3], pos[7], pos[5],
        pos[0], pos[2], pos[3], pos[1], pos[4], pos[5], pos[7], pos[6]
    };
    glm::vec3 nor[24];
    for (int i = 0; i < 24; i += 4) {
        for (int j = 0; j < 4; j++) {
            glm::vec3 curr = ver[i + j];
            glm::vec3 next = ver[i + (j + 1) % 4];
            glm::vec3 prev = ver[i + (j + 3) % 4];
            nor[i + j] = glm::normalize(glm::cross(next - curr, prev - curr));
        }
    }
    int size = info.size() / 16;
    for (int i = 0; i < 24; i++) {
        info.push_back(ver[i].x);
        info.push_back(ver[i].y);
        info.push_back(ver[i].z);
        info.push_back(1.f);
        info.push_back(nor[i].x);
        info.push_back(nor[i].y);
        info.push_back(nor[i].z);
        info.push_back(0.f);
        info.push_back(color.x); // r
        info.push_back(color.y); // g
        info.push_back(color.z); // b
        info.push_back(1.f);     // a
        info.push_back(0.f);     // u
        info.push_back(0.f);     // v
        info.push_back(5.f);     // cos
        info.push_back(0.f);     // anim
    }
    for (int i = 0; i < 24; i += 4) {
        idx.push_back(size + i);
        idx.push_back(size + i + 1);
        idx.push_back(size + i + 2);
        idx.push_back(size + i);
        idx.push_back(size + i + 2);
        idx.push_back(size + i + 3);
    }
}

void Hexahedron::stretch(int a, int b, float ratio) {
    if (a < 0 || b < 0 || a > 7 || b > 7) { return; }
    glm::vec3 mid = (pos[a] + pos[b]) / 2.f;
    pos[a] = mid + (pos[a] - mid) * ratio;
    pos[b] = mid + (pos[b] - mid) * ratio;
}

void Hexahedron::offset(int move, int base, float ratio) {
    if (move < 0 || base < 0 || move > 7 || base > 7) { return; }
    pos[move] = pos[base] + (pos[move] - pos[base]) * ratio;
}

void BodyPart::create() {
    if (m_boxs.size() <= 0) {
        return;
    }
    std::vector<GLuint> idx;
    std::vector<float> info;
    for (unsigned int i = 0; i < m_boxs.size(); i++) {
        m_boxs[i].populate(idx, info);
    }
    count0 = idx.size();
    count1 = 0;
    generateIdx0();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx0);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                          idx.size() * sizeof(GLuint),
                          idx.data(), GL_STATIC_DRAW);
    generateVer0();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVer0);
    context->glBufferData(GL_ARRAY_BUFFER,
                          info.size() * sizeof(float),
                          info.data(), GL_STATIC_DRAW);
}

glm::mat4 BodyPart::trans() const {
    return glm::translate(m_position) *
           glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z) *
           glm::scale(m_scale);
}

BodyPart* NPC::partAt(unsigned int index) {
    if (index >= m_parts.size()) {
        return nullptr;
    } else {
        return &(m_parts[index]);
    }
}

glm::mat4 NPC::partTrans(unsigned int index) const {
    if (index >= m_parts.size()) {
        return glm::translate(m_position) *
               glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z);
    } else if (m_parts[index].m_parent < 0 ||
               m_parts[index].m_parent >= (int)(m_parts.size())) {
        return glm::translate(m_position) *
               glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z) *
               m_parts[index].trans();
    } else {
        return glm::translate(m_position) *
               glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z) *
               m_parts[m_parts[index].m_parent].trans() *
               m_parts[index].trans();
    }
}

void NPC::create() {
    for (unsigned int i = 0; i < m_parts.size(); i++) {
        m_parts[i].create();
    }
}

bool NPC::notCollidePosX(const glm::mat4 &newtrans) const {
    glm::vec4 p0f = newtrans * glm::vec4(m_collider.pos[1], 1.f);
    int p0[3] = {(int)floorf(p0f.x), (int)floorf(p0f.y), (int)floorf(p0f.z)};
    glm::vec4 p1f = newtrans * glm::vec4(m_collider.pos[5], 1.f);
    int p1[3] = {(int)floorf(p1f.x), (int)floorf(p1f.y), (int)floorf(p1f.z)};
    glm::vec4 p2f = newtrans * glm::vec4(m_collider.pos[3], 1.f);
    int p2[3] = {(int)floorf(p2f.x), (int)floorf(p2f.y), (int)floorf(p2f.z)};
    glm::vec4 p3f = newtrans * glm::vec4(m_collider.pos[7], 1.f);
    int p3[3] = {(int)floorf(p3f.x), (int)floorf(p3f.y), (int)floorf(p3f.z)};
    return m_terrain->hasChunk(p0[0], p0[2]) && m_terrain->hasChunk(p1[0], p1[2]) &&
           m_terrain->hasChunk(p2[0], p2[2]) && m_terrain->hasChunk(p3[0], p3[2]) &&
           !Chunk::isCollidable(m_terrain->getBlockAt(p0[0], p0[1], p0[2])) &&
           !Chunk::isCollidable(m_terrain->getBlockAt(p1[0], p1[1], p1[2])) &&
           !Chunk::isCollidable(m_terrain->getBlockAt(p2[0], p2[1], p2[2])) &&
           !Chunk::isCollidable(m_terrain->getBlockAt(p3[0], p3[1], p3[2]));
}

glm::vec3 NPC::vecMoveAlongX(float amount) const {
    glm::vec4 vector4 = glm::eulerAngleY(m_rotation.y) * glm::vec4(1.f, 0.f, 0.f, 0.f);
    return amount * glm::vec3(vector4);
}

bool NPC::canMoveAlongX(float amount) const {
    glm::vec3 newposition = m_position + vecMoveAlongX(amount);
    glm::mat4 newtrans = glm::translate(newposition) *
                         glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z);
    return notCollidePosX(newtrans);
}

bool NPC::canPassAlongX(float amount) const {
    glm::vec3 newposition = m_position + vecMoveAlongX(amount) +
                            glm::vec3(0.f, jumpHeight(), 0.f);
    glm::mat4 newtrans = glm::translate(newposition) *
                         glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z);
    return notCollidePosX(newtrans);
}

bool NPC::canLiveAlongX(float amount, BiomeType biomeA, BiomeType biomeB) const {
    glm::vec3 newposition = m_position + vecMoveAlongX(amount);
    Biome biome((int)floorf(newposition.x), (int)floorf(newposition.z));
    BiomeType biomeType = biome.getBiome();
    return biomeType == biomeA || biomeType == biomeB;
}

void NPC::doMoveAlongX(float amount) {
    m_position += vecMoveAlongX(amount);
}

void NPC::doJump(float amount) {
    m_position[1] += amount;
}

void NPC::doRotateAlongY(float angle) {
    m_rotation[1] += angle;
}

bool NPC::appendGravity(float deltaTime) {
    glm::vec3 newposition = m_position +
                            glm::vec3(0.f, -deltaTime / 1000.f * fallSpeed(), 0.f);
    glm::mat4 newtrans = glm::translate(newposition) *
                         glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z);
    glm::vec4 p0f = newtrans * glm::vec4(m_collider.pos[0], 1.f);
    int p0[3] = {(int)floorf(p0f.x), (int)floorf(p0f.y), (int)floorf(p0f.z)};
    glm::vec4 p1f = newtrans * glm::vec4(m_collider.pos[1], 1.f);
    int p1[3] = {(int)floorf(p1f.x), (int)floorf(p1f.y), (int)floorf(p1f.z)};
    glm::vec4 p2f = newtrans * glm::vec4(m_collider.pos[4], 1.f);
    int p2[3] = {(int)floorf(p2f.x), (int)floorf(p2f.y), (int)floorf(p2f.z)};
    glm::vec4 p3f = newtrans * glm::vec4(m_collider.pos[5], 1.f);
    int p3[3] = {(int)floorf(p3f.x), (int)floorf(p3f.y), (int)floorf(p3f.z)};
    if (Chunk::isCollidable(m_terrain->getBlockAt(p0[0], p0[1], p0[2])) ||
        Chunk::isCollidable(m_terrain->getBlockAt(p1[0], p1[1], p1[2])) ||
        Chunk::isCollidable(m_terrain->getBlockAt(p2[0], p2[1], p2[2])) ||
        Chunk::isCollidable(m_terrain->getBlockAt(p3[0], p3[1], p3[2])) ||
        m_terrain->getBlockAt(p0[0], (int)floorf(p0f.y + floatHeight()), p0[2]) == WATER ||
        m_terrain->getBlockAt(p1[0], (int)floorf(p1f.y + floatHeight()), p1[2]) == WATER ||
        m_terrain->getBlockAt(p2[0], (int)floorf(p2f.y + floatHeight()), p2[2]) == WATER ||
        m_terrain->getBlockAt(p3[0], (int)floorf(p3f.y + floatHeight()), p3[2]) == WATER) {
        return false;
    } else {
        m_position = newposition;
        return true;
    }
}

void NPC::wander(float deltaTime, float totalTime,
            BiomeType biomeA, BiomeType biomeB) {
    float seed = noise::rand2D(noise::fractf(totalTime) * 111.f, deltaTime,
                               123.4f, 345.6f, 456.7f) * 111.f;
    if (m_amount < 0.f) {
        if (m_mission == MOVE) {
            m_amount = -1.5f + 3.f * noise::rand1D(seed);
            if (m_amount < 0) {
                m_amount = -m_amount;
                m_mission = ROTR;
            } else {
                m_mission = ROTL;
            }
        } else {
            m_mission = MOVE;
            m_amount = 3.f + 2.f * noise::rand1D(seed);
        }
    } else if (m_mission == IDLE) {
        m_amount -= deltaTime / 1000.f;
    } else if (m_mission == MOVE) {
        float moveAmount = deltaTime / 1000.f * moveSpeed();
        if (!canLiveAlongX(moveAmount, biomeA, biomeB)) {
            if (appendGravity(deltaTime)) {
                m_blendw -= deltaTime / 1000.f * blendSpeed();
                m_blendt -= deltaTime / 1000.f * blendSpeed();
                m_blendj += deltaTime / 1000.f * blendSpeed();
                m_blenduj -= deltaTime / 1000.f * blendSpeed();
            } else {
                m_amount = 3.f;
                if (noise::rand1D(seed) < 0.5f) {
                    m_mission = ROTL;
                } else {
                    m_mission = ROTR;
                }
            }
        } else if (canMoveAlongX(moveAmount)) {
            if (appendGravity(deltaTime)) {
                doMoveAlongX(moveAmount);
                m_blendw -= deltaTime / 1000.f * blendSpeed();
                m_blendt -= deltaTime / 1000.f * blendSpeed();
                m_blendj += deltaTime / 1000.f * blendSpeed();
                m_blenduj -= deltaTime / 1000.f * blendSpeed();
            } else {
                moveAmount *= m_blendw;
                doMoveAlongX(moveAmount);
                m_amount -= moveAmount;
                m_blendw += deltaTime / 1000.f * blendSpeed();
                m_blendt -= deltaTime / 1000.f * blendSpeed();
                m_blendj -= deltaTime / 1000.f * blendSpeed();
            }
        } else if (canPassAlongX(moveAmount)) {
            m_mission = JUMP;
            m_amount = jumpHeight();
        } else {
            m_amount = 2.f;
            if (noise::rand1D(seed) < 0.5f) {
                m_mission = ROTL;
            } else {
                m_mission = ROTR;
            }
        }
    } else if (m_mission == ROTL) {
        m_blendlt = true;
        float rotateAmount = deltaTime / 1000.f * rotateSpeed() * m_blendt;
        doRotateAlongY(rotateAmount);
        m_amount -= rotateAmount;
        m_blendw -= deltaTime / 1000.f * blendSpeed();
        m_blendt += deltaTime / 1000.f * blendSpeed();
        m_blendj -= deltaTime / 1000.f * blendSpeed();
    } else if (m_mission == ROTR) {
        m_blendlt = false;
        float rotateAmount = deltaTime / 1000.f * rotateSpeed() * m_blendt;
        doRotateAlongY(-rotateAmount);
        m_amount -= rotateAmount;
        m_blendw -= deltaTime / 1000.f * blendSpeed();
        m_blendt += deltaTime / 1000.f * blendSpeed();
        m_blendj -= deltaTime / 1000.f * blendSpeed();
    } else if (m_mission == JUMP) {
        float jumpAmount = deltaTime / 1000.f * jumpSpeed() * m_blendj;
        doJump(jumpAmount);
        m_amount -= jumpAmount;
        m_blendw -= deltaTime / 1000.f * blendSpeed();
        m_blendt -= deltaTime / 1000.f * blendSpeed();
        m_blendj += deltaTime / 1000.f * blendSpeed();
        m_blenduj += deltaTime / 1000.f * blendSpeed();
    } else {
        m_mission = IDLE;
    }

    m_blendw = noise::clamp(m_blendw, 0.f, 1.f);
    m_blendt = noise::clamp(m_blendt, 0.f, 1.f);
    m_blendj = noise::clamp(m_blendj, 0.f, 1.f);
    m_blenduj = noise::clamp(m_blenduj, 0.f, 1.f);
    float blentTotal = m_blendw + m_blendt + m_blendj;
    if (blentTotal > 0.99f) {
        m_blendw /= blentTotal;
        m_blendt /= blentTotal;
        m_blendj /= blentTotal;
    }
    idle(totalTime, 1 - (m_blendw + m_blendt + m_blendj));
    walk(totalTime, m_blendw);
    turn(totalTime, m_blendt, m_blendlt);
    jump(totalTime, m_blendj, m_blenduj);
}

void Ghost::update(float deltaTime, float totalTime) {
    wander(deltaTime, totalTime, DARK, MOUNTAIN);
}

void Ghost::birth() {
    glm::vec3 white(1.f, 1.f, 1.f);
    glm::vec3 black(0.f, 0.f, 0.f);
    BodyPart head(m_context, glm::vec3(0.f, 2.f, 0.f));
    float x[5] = {-0.5f, -0.4f, 0.f, 0.4f, 0.5f};
    float y[5] = {-1.2f, -1.f, -0.6f, -0.35f, 0.f};
    float z[6] = {-0.5f, -0.25f, -0.125f, 0.125f, 0.25f, 0.5f};
    head.add(Hexahedron(x[0], x[2], y[1], y[4], z[0], z[5], white));
    head.add(Hexahedron(x[1], x[3], y[2], y[3], z[1], z[4], black));
    head.add(Hexahedron(x[0], x[4], y[0], y[4], z[0], z[1], white));
    head.add(Hexahedron(x[0], x[4], y[0], y[4], z[2], z[3], white));
    head.add(Hexahedron(x[0], x[4], y[0], y[4], z[4], z[5], white));
    head.add(Hexahedron(x[0], x[4], y[1], y[2], z[0], z[5], white));
    head.add(Hexahedron(x[0], x[4], y[3], y[4], z[0], z[5], white));
    m_parts.push_back(head);
}

void Ghost::idle(float totalTime, float blend) {
    if (m_parts.size() < 1) { return; }
    m_parts[0].m_rotation[0] = blend * 0.f;
    m_parts[0].m_scale[1] = blend * (1.f + 0.3f * sinf(totalTime / 300.f));
}

void Ghost::walk(float totalTime, float blend) {
    if (m_parts.size() < 1 || blend < 0.001f) { return; }
    m_parts[0].m_rotation[0] += blend * 0.f;
    m_parts[0].m_scale[1] += blend * (1.f + 0.3f * sinf(totalTime / 300.f));
}

void Ghost::turn(float totalTime, float blend, bool left) {
    if (m_parts.size() < 1 || blend < 0.001f) { return; }
    m_parts[0].m_rotation[0] += left ? blend * -0.2f : blend * 0.2f;
    m_parts[0].m_scale[1] += blend * (1.f + 0.1f * sinf(totalTime / 300.f));
}

void Ghost::jump(float totalTime, float blend, float) {
    if (m_parts.size() < 1 || blend < 0.001f) { return; }
    m_parts[0].m_rotation[0] += blend * 0.f;
    m_parts[0].m_scale[1] += blend * (1.2f + 0.2f * sinf(totalTime / 300.f));
}

void Penguin::update(float deltaTime, float totalTime) {
    wander(deltaTime, totalTime, FROZEN, TUNDRA);
}

void Penguin::birth() {
    glm::vec3 white(1.f, 1.f, 1.f);
    glm::vec3 black(0.f, 0.f, 0.f);
    glm::vec3 orange(0.7f, 0.3f, 0.f);
    BodyPart body(m_context, glm::vec3(0.f, 0.2f, 0.f));
    float x[5] = {-0.5f, 0.3f, 0.47f, 0.5f, 0.65f};
    float y[7] = {0.f, 0.02f, 0.7f, 0.82f, 0.87f, 0.95f, 1.2f};
    float z[8] = {0.f, 0.18f, 0.22f, 0.25f, 0.33f, 0.41f, 0.48f, 0.5f};
    body.add(Hexahedron(-x[2], x[2], y[1], y[5], -z[6], z[6], white)); // belly
    body.add(Hexahedron(x[0], x[1], y[0], y[6], -z[7], z[7], black));  // back
    body.add(Hexahedron(x[0], x[3], y[5], y[6], -z[7], z[7], black));  // hair
    Hexahedron bang(x[1], x[3], y[3], y[5], -z[2], z[2], black);       // bang
    bang.stretch(1, 5, z[1] / z[2]);
    bang.stretch(0, 4, z[1] / z[2]);
    body.add(bang);
    body.add(Hexahedron(x[2], x[4], y[2], y[3], -z[3], z[3], orange)); // mouth
    body.add(Hexahedron(x[2], x[3], y[2], y[4], z[4], z[5], black));   // left eye
    body.add(Hexahedron(x[2], x[3], y[2], y[4], -z[4], -z[5], black)); // right eye
    m_parts.push_back(body);
    float footxmin = -0.3f;
    float footxmax = 0.55f;
    float footthick = 0.13f;
    float footwidth = 0.18f;
    BodyPart leftfoot(m_context, glm::vec3(0.f, 0.f, 0.f));
    leftfoot.add(Hexahedron(footxmin, footxmax, 0.f, footthick,
                            0.25f - footwidth, 0.25f + footwidth, orange));
    m_parts.push_back(leftfoot);
    BodyPart rigtfoot(m_context, glm::vec3(0.f, 0.f, 0.f));
    rigtfoot.add(Hexahedron(footxmin, footxmax, 0.f, footthick,
                            -0.25f - footwidth, -0.25f + footwidth, orange));
    m_parts.push_back(rigtfoot);
    float handpivotY = 0.7f;
    float handpivotZ = 0.52f;
    float handrotX = 0.5f;
    float handlength = 0.55f;
    float handthick = 0.12f;
    float handwidth = 0.2f;
    BodyPart lefthand(m_context, glm::vec3(0.f, handpivotY, handpivotZ),
                      glm::vec3(-handrotX, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f),
                      0); // body is parent
    lefthand.add(Hexahedron(-handwidth, handwidth, -handlength, 0.f,
                            0, handthick, black));
    m_parts.push_back(lefthand);
    BodyPart rigthand(m_context, glm::vec3(0.f, handpivotY, -handpivotZ),
                      glm::vec3(handrotX, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f),
                      0); // body is parent
    rigthand.add(Hexahedron(-handwidth, handwidth, -handlength, 0.f,
                            -handthick, 0, black));
    m_parts.push_back(rigthand);
}

void Penguin::idle(float, float blend) {
    if (m_parts.size() < 5) { return; }
    m_parts[0].m_position[1] = blend * 0.2f;  // body
    m_parts[0].m_rotation[0] = blend * 0.f;   // body
    m_parts[1].m_position[0] = blend * 0.f;   // left foot
    m_parts[1].m_position[1] = blend * 0.f;   // left foot
    m_parts[1].m_rotation[1] = blend * 0.f;   // left foot
    m_parts[2].m_position[0] = blend * 0.f;   // right foot
    m_parts[2].m_position[1] = blend * 0.f;   // right foot
    m_parts[2].m_rotation[1] = blend * 0.f;   // right foot
    m_parts[3].m_rotation[0] = blend * -0.5f; // left hand
    m_parts[4].m_rotation[0] = blend * 0.5f;  // right hand
}

void Penguin::walk(float totalTime, float blend) {
    if (m_parts.size() < 5 || blend < 0.001f) { return; }
    float frequency = 6.f;
    float x = totalTime / 1000.f * frequency;
    float dx = x - floorf(x / (2 * PI)) * 2 * PI;
    float k = moveSpeed() / frequency;
    float sin = sinf(x);
    m_parts[0].m_position[1] += blend * (0.2f + 0.1f + 0.1f * sinf(x * 2.f - PI / 2.f));
    m_parts[0].m_rotation[0] += blend * (0.f + 0.3f * sin);
    if (sin > 0) {
        m_parts[1].m_position[0] += blend * (-k * dx + k * PI * 0.5f);
        m_parts[1].m_position[1] += blend * 0.f;
        m_parts[2].m_position[0] += blend * (k * dx - k * PI * 0.5f);
        m_parts[2].m_position[1] += blend * 0.25f * sin;
    } else {
        m_parts[1].m_position[0] += blend * (k * dx - k * PI * 1.5f);
        m_parts[1].m_position[1] += blend * -0.25f * sin;
        m_parts[2].m_position[0] += blend * (-k * dx + k * PI * 1.5f);
        m_parts[2].m_position[1] += blend * 0.f;
    }
    m_parts[1].m_rotation[1] += blend * 0.f;
    m_parts[2].m_rotation[1] += blend * 0.f;
    m_parts[3].m_rotation[0] += blend * -0.5f;
    m_parts[4].m_rotation[0] += blend * 0.5f;
}

void Penguin::turn(float totalTime, float blend, bool left) {
    if (m_parts.size() < 5 || blend < 0.001f) { return; }
    float frequency = 6.f;
    float x = totalTime / 1000.f * frequency;
    float dx = x - floorf(x / (2 * PI)) * 2 * PI;
    float k = rotateSpeed() / frequency;
    if (!left) { k *= -1; }
    float sin = sinf(x);
    m_parts[0].m_position[1] += blend * (0.2f + 0.1f + 0.1f * sinf(x * 2.f - PI / 2.f));
    m_parts[0].m_rotation[0] += blend * (0.f + 0.3f * sin);
    m_parts[1].m_position[0] += blend * 0.f;
    m_parts[2].m_position[0] += blend * 0.f;
    if (sin > 0) {
        m_parts[1].m_position[1] += blend * 0.f;
        m_parts[1].m_rotation[1] += blend * (-k * dx + k * PI * 0.5f);
        m_parts[2].m_position[1] += blend * 0.25f * sin;
        m_parts[2].m_rotation[1] += blend * (k * dx - k * PI * 0.5f);
    } else {
        m_parts[1].m_position[1] += blend * -0.25f * sin;
        m_parts[1].m_rotation[1] += blend * (k * dx - k * PI * 1.5f);
        m_parts[2].m_position[1] += blend * 0.f;
        m_parts[2].m_rotation[1] += blend * (-k * dx + k * PI * 1.5f);
    }
    m_parts[3].m_rotation[0] += blend * -0.5f;
    m_parts[4].m_rotation[0] += blend * 0.5f;
}

void Penguin::jump(float totalTime, float blend, float up) {
    if (m_parts.size() < 5 || blend < 0.001f) { return; }
    m_parts[0].m_position[1] += blend * 0.2f;
    m_parts[0].m_rotation[0] += blend * 0.f;
    m_parts[1].m_position[0] += blend * 0.f;
    m_parts[1].m_position[1] += blend * 0.f;
    m_parts[1].m_rotation[1] += blend * 0.f;
    m_parts[2].m_position[0] += blend * 0.f;
    m_parts[2].m_position[1] += blend * 0.f;
    m_parts[2].m_rotation[1] += blend * 0.f;
    m_parts[3].m_rotation[0] += blend * (-0.5f - 0.3f +
                                         up * (0.8f * sinf(totalTime / 50.f)) +
                                         (1 - up) * -0.8f);
    m_parts[4].m_rotation[0] += blend * (0.5f + 0.3f -
                                         up * (0.8f * sinf(totalTime / 50.f)) -
                                         (1 - up) * -0.8f);
}

void Fish::update(float deltaTime, float totalTime) {
    wander(deltaTime, totalTime, JUNGLE, PLAIN);
}

void Fish::birth() {
    glm::vec3 orange(0.9f, 0.4f, 0.f);
    glm::vec3 black(0.f, 0.f, 0.f);
    BodyPart body(m_context, glm::vec3(0.2f, 0.25f, 0.f));
    float x[6] = {-0.35, -0.27, -0.13, 0.18, 0.24, 0.3};
    float y[5] = {0.f, 0.22f, 0.30f, 0.4f, 0.6f};
    float z[4] = {0.f, 0.03f, 0.15f, 0.17f};
    body.add(Hexahedron(x[0], x[5], y[0], y[3], -z[2], z[2], orange)); // body
    body.add(Hexahedron(x[3], x[4], y[1], y[2], z[2], z[3], black));   // left eye
    body.add(Hexahedron(x[3], x[4], y[1], y[2], -z[3], -z[2], black)); // right eye
    Hexahedron backfin(x[1], x[3], y[3], y[4], -z[1], z[1], orange);   // back fin
    backfin.offset(3, 2, (x[2] - x[1]) / (x[3] - x[1]));
    backfin.offset(7, 6, (x[2] - x[1]) / (x[3] - x[1]));
    body.add(backfin);
    m_parts.push_back(body);
    float tailpivotX = -0.3f;
    float taillength = 0.4f;
    float tailwidth = 0.2f;
    float tailthick = 0.03f;
    float tailstretch = 0.3f;
    BodyPart tailfin(m_context, glm::vec3(tailpivotX, tailwidth, 0.f),
                     glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f),
                     0); // body is parent
    Hexahedron tail(-taillength, 0.f, -tailwidth, tailwidth,
                    -tailthick, tailthick, orange);
    tail.stretch(1, 3, tailstretch);
    tail.stretch(7, 5, tailstretch);
    tailfin.add(tail);
    m_parts.push_back(tailfin);
    float sidePivotX = 0.08f;
    float sidePivotY = 0.2f;
    float sidePivotZ = 0.2f;
    float sidelength = 0.4f;
    float sidewidth = 0.1f;
    float sidethick = 0.03f;
    float siderotY = 0.2f;
    BodyPart leftfin(m_context, glm::vec3(sidePivotX, sidePivotY, sidePivotZ),
                     glm::vec3(0.f, siderotY, 0.f), glm::vec3(1.f, 1.f, 1.f),
                     0); // body is parent
    leftfin.add(Hexahedron(-sidelength, 0.f, -sidewidth, sidewidth,
                           -sidethick, sidethick, orange));
    m_parts.push_back(leftfin);
    BodyPart rigtfin(m_context, glm::vec3(sidePivotX, sidePivotY, -sidePivotZ),
                     glm::vec3(0.f, -siderotY, 0.f), glm::vec3(1.f, 1.f, 1.f),
                     0); // body is parent
    rigtfin.add(Hexahedron(-sidelength, 0.f, -sidewidth, sidewidth,
                           -sidethick, sidethick, orange));
    m_parts.push_back(rigtfin);
}

void Fish::idle(float totalTime, float blend) {
    if (m_parts.size() < 4) { return; }
    m_parts[1].m_rotation[1] = blend * (0.5f * sinf(totalTime / 150.f));
    m_parts[2].m_rotation[1] = blend * (0.2f + 0.2f * sinf(totalTime / 200.f));
    m_parts[3].m_rotation[1] = blend * (-0.2f - 0.2f * sinf(totalTime / 200.f));
}

void Fish::walk(float totalTime, float blend) {
    if (m_parts.size() < 4 || blend < 0.001f) { return; }
    m_parts[1].m_rotation[1] += blend * (0.5f * sinf(totalTime / 150.f));
    m_parts[2].m_rotation[1] += blend * (0.2f + 0.2f * sinf(totalTime / 200.f));
    m_parts[3].m_rotation[1] += blend * (-0.2f - 0.2f * sinf(totalTime / 200.f));
}

void Fish::turn(float totalTime, float blend, bool left) {
    if (m_parts.size() < 4 || blend < 0.001f) { return; }
    m_parts[1].m_rotation[1] += blend * (left ? -0.5f : 0.5f);
    m_parts[2].m_rotation[1] += blend * (0.1f + 0.1f * sinf(totalTime / 200.f));
    m_parts[3].m_rotation[1] += blend * (-0.1f - 0.1f * sinf(totalTime / 200.f));
}

void Fish::jump(float totalTime, float blend, float) {
    if (m_parts.size() < 4 || blend < 0.001f) { return; }
    m_parts[1].m_rotation[1] += blend * (0.5f * sinf(totalTime / 150.f));
    m_parts[2].m_rotation[1] += blend * (0.2f + 0.2f * sinf(totalTime / 200.f));
    m_parts[3].m_rotation[1] += blend * (-0.2f - 0.2f * sinf(totalTime / 200.f));
}

void Sheep::update(float deltaTime, float totalTime) {
    wander(deltaTime, totalTime, PLAIN, JUNGLE);
}

void Sheep::birth() {
    glm::vec3 white(1.f, 1.f, 1.f);
    glm::vec3 black(0.f, 0.f, 0.f);
    glm::vec3 pink(0.9f, 0.8f, 0.6f);
    float bodypivotY = 0.25f;
    BodyPart body(m_context, glm::vec3(0.f, bodypivotY, 0.f));
    float bodylength = 0.7f;
    float bodywidth = 0.5f;
    float bodyheight = 0.9f;
    float taillength = 0.4f;
    float tailwidth = 0.1f;
    float tailpivotY = 0.8f;
    body.add(Hexahedron(-bodylength, bodylength, 0.f, bodyheight,
                        -bodywidth, bodywidth, white)); // body
    body.add(Hexahedron(-bodylength - tailwidth * 2, -bodylength, tailpivotY - taillength,
                        tailpivotY, -tailwidth, tailwidth, white)); // tail
    float headpivotX = 0.4f;
    m_parts.push_back(body);
    BodyPart head(m_context, glm::vec3(headpivotX, bodypivotY + bodyheight, 0.f));
    float x[4] = {-0.4f, -0.15f, 0.38f, 0.4f};
    float y[6] = {0.f, 0.f, 0.25f, 0.4f, 0.5f, 0.8f};
    float z[5] = {0.f, 0.13f, 0.17f, 0.36f, 0.4f};
    head.add(Hexahedron(x[1], x[2], y[1], y[4], -z[3], z[3], pink));  // face
    head.add(Hexahedron(x[0], x[1], y[0], y[5], -z[4], z[4], white)); // braid
    head.add(Hexahedron(x[0], x[3], y[4], y[5], -z[4], z[4], white)); // hair
    head.add(Hexahedron(x[2], x[3], y[2], y[3], z[1], z[2], black));  // left eye
    head.add(Hexahedron(x[2], x[3], y[2], y[3], -z[2], -z[1], black));// right eye
    m_parts.push_back(head);
    float earpivotX = 0.1f;
    float earpivotY = 0.4f;
    float earpivotZ = 0.4f;
    float earrotX = 0.6f;
    float earlenght = 0.5f;
    float earwidth = 0.12f;
    float earthich = 0.03f;
    BodyPart leftear(m_context, glm::vec3(earpivotX, earpivotY, earpivotZ),
                     glm::vec3(-earrotX, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f),
                     1); // head is parent
    leftear.add(Hexahedron(-earwidth, earwidth, -earlenght, 0.f, -earthich, earthich, pink));
    m_parts.push_back(leftear);
    BodyPart rigtear(m_context, glm::vec3(earpivotX, earpivotY, -earpivotZ),
                     glm::vec3(earrotX, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f),
                     1); // head is parent
    rigtear.add(Hexahedron(-earwidth, earwidth, -earlenght, 0.f, -earthich, earthich, pink));
    m_parts.push_back(rigtear);
    float footpivotX = 0.4f;
    float footpivotZ = 0.25f;
    float footwidth = 0.2f;
    float footheight = 0.25f;
    BodyPart lffoot(m_context, glm::vec3(0.f, 0.f, 0.f));
    lffoot.add(Hexahedron(footpivotX - footwidth, footpivotX + footwidth, 0.f, footheight,
                          footpivotZ - footwidth, footpivotZ + footwidth, pink));
    m_parts.push_back(lffoot);
    BodyPart rffoot(m_context, glm::vec3(0.f, 0.f, 0.f));
    rffoot.add(Hexahedron(footpivotX - footwidth, footpivotX + footwidth, 0.f, footheight,
                          -footpivotZ - footwidth, -footpivotZ + footwidth, pink));
    m_parts.push_back(rffoot);
    BodyPart lbfoot(m_context, glm::vec3(0.f, 0.f, 0.f));
    lbfoot.add(Hexahedron(-footpivotX - footwidth, -footpivotX + footwidth, 0.f, footheight,
                          footpivotZ - footwidth, footpivotZ + footwidth, pink));
    m_parts.push_back(lbfoot);
    BodyPart rbfoot(m_context, glm::vec3(0.f, 0.f, 0.f));
    rbfoot.add(Hexahedron(-footpivotX - footwidth, -footpivotX + footwidth, 0.f, footheight,
                          -footpivotZ - footwidth, -footpivotZ + footwidth, pink));
    m_parts.push_back(rbfoot);
}

void Sheep::idle(float, float blend) {
    if (m_parts.size() < 8) { return; }
    m_parts[0].m_position[1] = blend * 0.25f;
    m_parts[1].m_position[1] = blend * 1.15f;
    m_parts[2].m_rotation[0] = blend * -0.6f;
    m_parts[3].m_rotation[0] = blend * 0.6f;
    m_parts[4].m_position[0] = blend * 0.f;
    m_parts[4].m_position[1] = blend * 0.f;
    m_parts[4].m_rotation[1] = blend * 0.f;
    m_parts[5].m_position[0] = blend * 0.f;
    m_parts[5].m_position[1] = blend * 0.f;
    m_parts[5].m_rotation[1] = blend * 0.f;
    m_parts[6].m_position[0] = blend * 0.f;
    m_parts[6].m_position[1] = blend * 0.f;
    m_parts[6].m_rotation[1] = blend * 0.f;
    m_parts[7].m_position[0] = blend * 0.f;
    m_parts[7].m_position[1] = blend * 0.f;
    m_parts[7].m_rotation[1] = blend * 0.f;
}

void Sheep::walk(float totalTime, float blend) {
    if (m_parts.size() < 8 || blend < 0.001f) { return; }
    float frequency = 8.f;
    float x = totalTime / 1000.f * frequency;
    float dx = x - floorf(x / (2 * PI)) * 2 * PI;
    float k = moveSpeed() / frequency;
    float sin = sinf(x);
    float sin2 = sinf(x * 2.f - PI / 2.f);
    m_parts[0].m_position[1] += blend * (0.25f + 0.1f + 0.1f * sin2);
    m_parts[1].m_position[1] += blend * (1.15f + 0.1f + 0.1f * sin2);
    m_parts[2].m_rotation[0] += blend * (-0.6f + 0.1f * sin2);
    m_parts[3].m_rotation[0] += blend * (0.6f - 0.1f * sin2);
    if (sin > 0) {
        m_parts[4].m_position[0] += blend * (-k * dx + k * PI * 0.5f);
        m_parts[4].m_position[1] += blend * 0.f;
        m_parts[5].m_position[0] += blend * (k * dx - k * PI * 0.5f);
        m_parts[5].m_position[1] += blend * 0.25f * sin;
        m_parts[6].m_position[0] += blend * (k * dx - k * PI * 0.5f);
        m_parts[6].m_position[1] += blend * 0.25f * sin;
        m_parts[7].m_position[0] += blend * (-k * dx + k * PI * 0.5f);
        m_parts[7].m_position[1] += blend * 0.f;
    } else {
        m_parts[4].m_position[0] += blend * (k * dx - k * PI * 1.5f);
        m_parts[4].m_position[1] += blend * -0.25f * sin;
        m_parts[5].m_position[0] += blend * (-k * dx + k * PI * 1.5f);
        m_parts[5].m_position[1] += blend * 0.f;
        m_parts[6].m_position[0] += blend * (-k * dx + k * PI * 1.5f);
        m_parts[6].m_position[1] += blend * 0.f;
        m_parts[7].m_position[0] += blend * (k * dx - k * PI * 1.5f);
        m_parts[7].m_position[1] += blend * -0.25f * sin;
    }
    m_parts[4].m_rotation[1] += blend * 0.f;
    m_parts[5].m_rotation[1] += blend * 0.f;
    m_parts[6].m_rotation[1] += blend * 0.f;
    m_parts[7].m_rotation[1] += blend * 0.f;
}

void Sheep::turn(float totalTime, float blend, bool left) {
    if (m_parts.size() < 8 || blend < 0.001f) { return; }
    float frequency = 8.f;
    float x = totalTime / 1000.f * frequency;
    float dx = x - floorf(x / (2 * PI)) * 2 * PI;
    float k = rotateSpeed() / frequency;
    if (!left) { k *= -1; }
    float sin = sinf(x);
    float sin2 = sinf(x * 2.f - PI / 2.f);
    m_parts[0].m_position[1] += blend * (0.25f + 0.1f + 0.1f * sin2);
    m_parts[1].m_position[1] += blend * (1.15f + 0.1f + 0.1f * sin2);
    m_parts[2].m_rotation[0] += blend * (-0.6f + 0.1f * sin2);
    m_parts[3].m_rotation[0] += blend * (0.6f - 0.1f * sin2);
    if (sin > 0) {
        m_parts[4].m_rotation[1] += blend * (-k * dx + k * PI * 0.5f);
        m_parts[4].m_position[1] += blend * 0.f;
        m_parts[5].m_rotation[1] += blend * (k * dx - k * PI * 0.5f);
        m_parts[5].m_position[1] += blend * 0.25f * sin;
        m_parts[6].m_rotation[1] += blend * (k * dx - k * PI * 0.5f);
        m_parts[6].m_position[1] += blend * 0.25f * sin;
        m_parts[7].m_rotation[1] += blend * (-k * dx + k * PI * 0.5f);
        m_parts[7].m_position[1] += blend * 0.f;
    } else {
        m_parts[4].m_rotation[1] += blend * (k * dx - k * PI * 1.5f);
        m_parts[4].m_position[1] += blend * -0.25f * sin;
        m_parts[5].m_rotation[1] += blend * (-k * dx + k * PI * 1.5f);
        m_parts[5].m_position[1] += blend * 0.f;
        m_parts[6].m_rotation[1] += blend * (-k * dx + k * PI * 1.5f);
        m_parts[6].m_position[1] += blend * 0.f;
        m_parts[7].m_rotation[1] += blend * (k * dx - k * PI * 1.5f);
        m_parts[7].m_position[1] += blend * -0.25f * sin;
    }
    m_parts[4].m_position[0] += blend * 0.f;
    m_parts[5].m_position[0] += blend * 0.f;
    m_parts[6].m_position[0] += blend * 0.f;
    m_parts[7].m_position[0] += blend * 0.f;
}

void Sheep::jump(float, float blend, float up) {
    if (m_parts.size() < 8 || blend < 0.001f) { return; }
    m_parts[0].m_position[1] += blend * (0.25f + 0.2f);
    m_parts[1].m_position[1] += blend * (1.15f + 0.4f);
    m_parts[2].m_rotation[0] += blend * (noise::mix(-0.9f, -0.3f, up));
    m_parts[3].m_rotation[0] += blend * (noise::mix(0.9f, 0.3f, up));
    m_parts[4].m_position[0] += blend * 0.f;
    m_parts[4].m_position[1] += blend * 0.f;
    m_parts[4].m_rotation[1] += blend * 0.f;
    m_parts[5].m_position[0] += blend * 0.f;
    m_parts[5].m_position[1] += blend * 0.f;
    m_parts[5].m_rotation[1] += blend * 0.f;
    m_parts[6].m_position[0] += blend * 0.f;
    m_parts[6].m_position[1] += blend * 0.f;
    m_parts[6].m_rotation[1] += blend * 0.f;
    m_parts[7].m_position[0] += blend * 0.f;
    m_parts[7].m_position[1] += blend * 0.f;
    m_parts[7].m_rotation[1] += blend * 0.f;
}

void NPCSystem::birthNPC(const Rect16 &scope) {
    // get xz coord
    int x = scope.xmid();
    int z = scope.zmid();
    // get random value
    float seed = noise::rand2D((float)(x % 1024), (float)(z % 1024),
                               123.4f, 345.6f, 456.7f) * 111.f;
    float rand1 = noise::rand1D(seed);
    float rand2 = noise::rand1D(seed + 123.4);
    // get top height
    int top = 255;
    for (int y = 255; y >= 0; y--) {
        if (Chunk::isCollidable(m_terrain->getBlockAt(x, y, z))) {
            top = y;
            break;
        }
    }
    // compute npc root position
    float npcx = x + 0.5f;
    float npcy = top + 1.f;
    float npcz = z + 0.5f;
    // get biome type
    Biome biome(x, z);
    BiomeType biomeType = biome.getBiome();
    // place npc based on biome type
    switch (biomeType) {
    case DARK:
    case MOUNTAIN:
        if (rand1 > 0.75f) {
            npcs.push_back(mkU<Ghost>(m_context, m_terrain,
                                      glm::vec3(npcx, npcy + 2.f, npcz),
                                      glm::vec3(0.f, rand2 * 6.283f, 0.f)));
        }
        break;
    case FROZEN:
    case TUNDRA:
        if (rand1 > 0.75f) {
            npcs.push_back(mkU<Penguin>(m_context, m_terrain,
                                        glm::vec3(npcx, npcy + 2.f, npcz),
                                        glm::vec3(0.f, rand2 * 6.283f, 0.f)));
        }
        break;
    case JUNGLE:
    case PLAIN:
        if (m_terrain->getBlockAt(x, 128, z) == WATER) {
            npcs.push_back(mkU<Fish>(m_context, m_terrain,
                                     glm::vec3(npcx, 128, npcz),
                                     glm::vec3(0.f, rand2 * 6.283f, 0.f)));
        } else if (biomeType == PLAIN && rand1 > 0.75f) {
            npcs.push_back(mkU<Sheep>(m_context, m_terrain,
                                      glm::vec3(npcx, npcy + 2.f, npcz),
                                      glm::vec3(0.f, rand2 * 6.283f, 0.f)));
        }
        break;
    default:
        break;
    }
}

void NPCSystem::update(float deltaTime) {
//#define PRINT_NPC_COUNT
#ifdef PRINT_NPC_COUNT
    std::cout << "NPC count: " << npcs.size() <<
                 "\tFPS: " << 1000.f / deltaTime << std::endl;
#endif
    m_totalTime += deltaTime;
    for (unsigned int i = 0; i < npcs.size(); i++) {
        npcs[i]->update(deltaTime, m_totalTime);
    }
}

void NPCSystem::destroy() {
    for (unsigned int i = 0; i < npcs.size(); i++) {
        NPC *npc = npcs[i].get();
        for (unsigned int j = 0; j < npc->size(); j++) {
            npc->partAt(j)->destroy();
        }
    }
}
