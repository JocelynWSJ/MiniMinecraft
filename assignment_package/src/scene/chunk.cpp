#include "chunk.h"

// openGL create
void Chunk::create() {
    //createCloud();
    ChunkCreateInfo info;
    populateInfo(&info);
    create(&info);
}

void Chunk::create(const ChunkCreateInfo *info) {
    if (info == nullptr) {
        return;
    }

    count0 = info->idx0.size();
    count1 = info->idx1.size();
    // Opaque pass
    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx0();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx0);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, info->idx0.size() * sizeof(GLuint), info->idx0.data(), GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generateVer0();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVer0);
    context->glBufferData(GL_ARRAY_BUFFER, info->opaque.size() * sizeof(float), info->opaque.data(), GL_STATIC_DRAW);

    // Transparent pass
    // Create a VBO on our GPU and store its handle in bufIdx

    if (info->idx1.size() > 0) {
        generateIdx1();
        // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
        // and that it will be treated as an element array buffer (since it will contain triangle indices)
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx1);
        // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
        // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
        context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, info->idx1.size() * sizeof(GLuint), info->idx1.data(), GL_STATIC_DRAW);

        // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
        // array buffers rather than element array buffers, as they store vertex attributes like position.
        generateVer1();
        context->glBindBuffer(GL_ARRAY_BUFFER, bufVer1);
        context->glBufferData(GL_ARRAY_BUFFER, info->transparency.size() * sizeof(float), info->transparency.data(), GL_STATIC_DRAW);
    }
}

// populate and create this and all neighbors
void Chunk::combinedCreate() {
    ChunkCreateInfo info;
    ChunkCreateInfo li;
    ChunkCreateInfo ri;
    ChunkCreateInfo fi;
    ChunkCreateInfo bi;
    populateInfo(&info);
    populateNeighbor(&li, &ri, &fi, &bi);
    updateSelf(&info);
    updateNeighbor(&li, &ri, &fi, &bi);
}

// populate chunk create info
void Chunk::populateInfo(ChunkCreateInfo *info) const {
    createCubes(info->opaque, info->transparency, info->idx0, info->idx1);
}

// populate chunk create info of neighbors
void Chunk::populateNeighbor(ChunkCreateInfo *li, ChunkCreateInfo *ri,
                             ChunkCreateInfo *fi, ChunkCreateInfo *bi) const {
    if (left != nullptr) {
        left->populateInfo(li);
    }
    if (right != nullptr) {
        right->populateInfo(ri);
    }
    if (front != nullptr) {
        front->populateInfo(fi);
    }
    if (back != nullptr) {
        back->populateInfo(bi);
    }
}

// recreate this chunk and all neighbors
void Chunk::updateSelf(const ChunkCreateInfo *info) {
    destroy();
    create(info);
}

// recreate all neighbors
void Chunk::updateNeighbor(const ChunkCreateInfo *li, const ChunkCreateInfo *ri,
                           const ChunkCreateInfo *fi, const ChunkCreateInfo *bi) {
    if (left != nullptr) {
        left->updateSelf(li);
    }
    if (right != nullptr) {
        right->updateSelf(ri);
    }
    if (front != nullptr) {
        front->updateSelf(fi);
    }
    if (back != nullptr) {
        back->updateSelf(bi);
    }
}

// get the blocktype located at that position in the chunk
BlockType Chunk::blockAt(int x, int y, int z) const {
    int index = x + y * 16 + z * 16 * 256;
    return m_blocks[index];
}

// set the blocktype located at that position in the Chunk
BlockType& Chunk::blockAt(int x, int y, int z) {
    int index = x + y * 16 + z * 16 * 256;
    return m_blocks[index];
}

// is empty or transparent
bool Chunk::isOpaqueType(BlockType type) {
    if (type == EMPTY || type == WATER || type == ICE || type == LAVA ||
        isCrossType(type) || type == CLOUD) {
        return false;
    }
    return true;
}

bool Chunk::isCollidable(BlockType type) {
    if (type == EMPTY || type == CLOUD || type == WATER || isCrossType(type)) {
        return false;
    }
    return true;
}

bool Chunk::isCrossType(BlockType type) {
    if (type == REDFLOWER || type == CROSSGRASS || type == MUSHROOM ||
        type == GREYMUSHROOM || type == BUSH || type == DEADBRANCH) {
        return true;
    }
    return false;
}

// set up vbo for all non-empty cubes in this chunk
void Chunk::createCubes(std::vector<float>& opaque,
                        std::vector<float>& transparency,
                        std::vector<GLuint>& idx0,
                        std::vector<GLuint>& idx1) const {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 16; k++) {
                if (blockAt(i, j, k) != EMPTY){
                    visitBlocks(i, j, k, opaque, transparency, idx0, idx1);
                }
            }
        }
    }
}

bool Chunk::isBlockOpaque(int i, int j, int k) const {
    BlockType type = blockAt(i, j, k);
    return isOpaqueType(type);
}

// get the index located at a given position in this chunk
int Chunk::getIndex(int x, int y, int z) const {
    return x + y * 16 + z * 16 * 256;
}

// determine whether a face should be painted
bool Chunk::shouldPaint(int x, int y, int z, FaceType face) const {
    if (isBlockOpaque(x, y, z)) {
        switch (face) {
        case LEFT:
            if ((x == 0 && (!left || !left->isBlockOpaque(15, y, z))) ||
                    (x > 0 && x < 16 && !isBlockOpaque(x - 1, y, z))) {
                return true;
            } else {
                return false;
            }
            break;
        case RIGHT:
            if ((x == 15 && (!right || !right->isBlockOpaque(0, y, z))) ||
                    (x >= 0 && x < 15 && !isBlockOpaque(x + 1, y, z))) {
                return true;
            } else {
                return false;
            }
            break;
        case BOTTOM:
            if (y == 0 || (y > 0 && y < 256 && !isBlockOpaque(x, y - 1, z))) {
                return true;
            } else {
                return false;
            }
            break;
        case TOP:
            if (y == 255 || (y >= 0 && y < 255 && !isBlockOpaque(x, y + 1, z))) {
                return true;
            } else {
                return false;
            }
            break;
        case BACK:
            if ((z == 0 && (!back || !back->isBlockOpaque(x, y, 15))) ||
                    (z > 0 && z < 16 && !isBlockOpaque(x, y, z - 1))) {
                return true;
            } else {
                return false;
            }
            break;
        case FRONT:
            if ((z == 15 && (!front || !front->isBlockOpaque(x, y, 0))) ||
                    (z >= 0 && z < 15 && !isBlockOpaque(x, y, z + 1))) {
                return true;
            } else {
                return false;
            }
            break;
        default:
            break;
        }

    } else {
        switch (face) {
        case LEFT:
            if ((x == 0 && (!left || left->blockAt(15, y, z) == EMPTY)) ||
                    (x > 0 && x < 16 && blockAt(x - 1, y, z) == EMPTY)) {
                return true;
            } else {
                return false;
            }
            break;
        case RIGHT:
            if ((x == 15 && (!right || right->blockAt(0, y, z) == EMPTY)) ||
                    (x >= 0 && x < 15 && blockAt(x + 1, y, z) == EMPTY)) {
                return true;
            } else {
                return false;
            }
            break;
        case BOTTOM:
            if (y == 0 ||
                    (y > 0 && y < 256 && blockAt(x, y - 1, z) == EMPTY)) {
                return true;
            } else {
                return false;
            }
            break;
        case TOP:
            if (y == 255 ||
                    (y >= 0 && y < 255 && blockAt(x, y + 1, z) == EMPTY)) {
                return true;
            } else {
                return false;
            }
            break;
        case BACK:
            if ((z == 0 && (!back || back->blockAt(x, y, 15) == EMPTY)) ||
                    (z > 0 && z < 16 && blockAt(x, y, z - 1) == EMPTY)) {
                return true;
            } else {
                return false;
            }
            break;
        case FRONT:
            if ((z == 15 && (!front || front->blockAt(x, y, 0) == EMPTY)) ||
                    (z >= 0 && z < 15 && blockAt(x, y, z + 1) == EMPTY)) {
                return true;
            } else {
                return false;
            }
            break;
        default:
            break;
        }
    }
    return false;
}

// visit neighboring blocks and set up vbo for a single block
void Chunk::visitBlocks(int x, int y, int z,
                        std::vector<float>& opaque,
                        std::vector<float>& transparency,
                        std::vector<GLuint>& idx0,
                        std::vector<GLuint>& idx1) const {
    BlockType type = blockAt(x, y, z);
    glm::vec4 currentPos = m_originPos + glm::vec4(x, y, z, 0);
    // if is a crossing decal
    if (isCrossType(type)) {
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(0, 0, 0.5, 0));
        pos.push_back(currentPos + glm::vec4(1, 0, 0.5, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 0.5, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 0.5, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, FRONT);
        pos.clear();
        pos.push_back(currentPos + glm::vec4(0.5, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(0.5, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(0.5, 1, 0, 0));
        pos.push_back(currentPos + glm::vec4(0.5, 1, 1, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, RIGHT);
        pos.clear();
        pos.push_back(currentPos + glm::vec4(1, 0, 0.5, 0));
        pos.push_back(currentPos + glm::vec4(0, 0, 0.5, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 0.5, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 0.5, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, BACK);
        pos.clear();
        pos.push_back(currentPos + glm::vec4(0.5, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(0.5, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(0.5, 1, 1, 0));
        pos.push_back(currentPos + glm::vec4(0.5, 1, 0, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, LEFT);
        return;
    }
    // if the adjancant block is empty, we need to render
    if (shouldPaint(x, y, z, LEFT)){
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(0, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(0, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 1, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 0, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, LEFT);
    }
    if (shouldPaint(x, y, z, RIGHT)) {
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(1, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(1, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 0, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 1, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, RIGHT);
    }
    if (shouldPaint(x, y, z, BOTTOM)) {
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(0, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(1, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(1, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(0, 0, 1, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, BOTTOM);
    }
    if (shouldPaint(x, y, z, TOP)) {
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(0, 1, 1, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 1, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 0, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 0, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, TOP);
    }
    if (shouldPaint(x, y, z, BACK)) {
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(1, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(0, 0, 0, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 0, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 0, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, BACK);
    }
    if (shouldPaint(x, y, z, FRONT)) {
        std::vector<glm::vec4> pos;
        pos.push_back(currentPos + glm::vec4(0, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(1, 0, 1, 0));
        pos.push_back(currentPos + glm::vec4(1, 1, 1, 0));
        pos.push_back(currentPos + glm::vec4(0, 1, 1, 0));
        addFace(pos, type, opaque, transparency, idx0, idx1, FRONT);
    }
}

// add face for a block
void Chunk::addFace(std::vector<glm::vec4> pos, BlockType type,
                    std::vector<float>& opaque,
                    std::vector<float>& transparency,
                    std::vector<GLuint>& idx0,
                    std::vector<GLuint>& idx1,
                    FaceType face) const {
    glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(pos[0])- glm::vec3(pos[1]),
                                      glm::vec3(pos[0])- glm::vec3(pos[2])));
    int size0 = opaque.size() / 16;
    int size1 = transparency.size() / 16;
    // vert -> pos1nor1col1uv1
    for (int i = 0; i < 4; i++){
        if (isOpaqueType(type)) {
            for (int j = 0; j < 4; j++) {
               opaque.push_back(pos[i][j]);
            }
            for (int j = 0; j < 3; j++) {
               opaque.push_back(normal[j]);
            }
            for (int j = 0; j < 5; j++) {
               opaque.push_back(0);
            }
            addUV(opaque, type, face, i);
            idx0.push_back(size0);
            idx0.push_back(size0 + 1);
            idx0.push_back(size0 + 2);
            idx0.push_back(size0);
            idx0.push_back(size0 + 2);
            idx0.push_back(size0 + 3);
        }
        else {
            for (int j = 0; j < 4; j++) {
               transparency.push_back(pos[i][j]);
            }
            for (int j = 0; j < 3; j++) {
               transparency.push_back(normal[j]);
            }
            for (int j = 0; j < 5; j++) {
               transparency.push_back(0);
            }
            addUV(transparency, type, face, i);
            idx1.push_back(size1);
            idx1.push_back(size1 + 1);
            idx1.push_back(size1 + 2);
            idx1.push_back(size1);
            idx1.push_back(size1 + 2);
            idx1.push_back(size1 + 3);
        }
    }
}

// add uv for a block
void Chunk::addUV(std::vector<float>& verts, BlockType type, FaceType face,
                  int i) const {
    float x = 0;
    float y = 0;
    float cosine = 0;
    int animated = 0;
    switch (type) {
    case GRASS:
        cosine = 5;
        if (face == TOP) {
            x = 8.01;
            y = 13.01;
        }
        else {
            x = 3.01;
            y = 15.01;
        }
        break;
    case DIRT:
        cosine = 5;
        x = 2.01;
        y = 15.01;
        break;
    case STONE:
        cosine = 3;
        x = 1.01;
        y = 15.01;
        break;
    case LAVA:
        animated = 1;
        cosine = 8;
        x = 14.01;
        y = 1.01;
        break;
    case WATER:
        animated = 1;
        cosine = 8;
        x = 14.01;
        y = 3.01;
        break;
    case SNOW:
        cosine = 5;
        if (face == TOP) {
            x = 2.01;
            y = 11.01;
        }
        else {
            x = 4.01;
            y = 11.01;
        }
        break;
    case BEDROCK:
        cosine = 3;
        x = 1.01;
        y = 14.01;
        break;
    case WOOD:
        cosine = 5;
        if (face == TOP) {
            x = 5.01;
            y = 14.01;
        }
        else {
            x = 4.01;
            y = 14.01;
        }
        break;
    case LEAF:
        cosine = 5;
        animated = 10;
        x = 5.01;
        y = 12.01;
        break;
    case ICE:
        cosine = 3;
        x = 3.01;
        y = 11.01;
        break;
    case REDFLOWER:
        cosine = 5;
        x = 12.01;
        y = 15.01;
        break;
    case CROSSGRASS:
        cosine = 5;
        x = 7.01;
        y = 13.01;
        break;
    case MUSHROOM:
        cosine = 5;
        x = 12.01;
        y = 14.01;
        break;
    case LAKEBOTTOM:
        cosine = 3;
        x = 2.01;
        y = 14.01;
        break;
    case SAND:
        cosine = 5;
        x = 0.01;
        y = 4.01;
        break;
    case EVIL:
        cosine = 5;
        x = 5.01;
        y = 13.01;
        break;
    case LEAFMOLD:
        cosine = 5;
        x = 4.01;
        y = 12.01;
        break;
    case FROZEDIRT:
        cosine = 5;
        if (face == TOP) {
            x = 14.01;
            y = 11.01;
        } else {
            x = 13.01;
            y = 11.01;
        }
        break;
    case GREYMUSHROOM:
        cosine = 5;
        x = 13.01;
        y = 14.01;
        break;
    case BUSH:
        cosine = 5;
        x = 15.01;
        y = 12.01;
        break;
    case DEADBRANCH:
        cosine = 5;
        x = 7.01;
        y = 12.01;
        break;
    case GOLD:
        cosine = 3;
        x = 0.01;
        y = 13.01;
        break;
    case COAL:
        cosine = 3;
        x = 2.01;
        y = 13.01;
        break;
    case RUBY:
        cosine = 3;
        x = 3.01;
        y = 12.01;
        break;
    case YELLOWROCK:
        cosine = 5;
        x = 2.01;
        y = 5.01;
        break;
    case ORANGEROCK:
        cosine = 5;
        x = 2.01;
        y = 2.01;
        break;
    case REDROCK:
        cosine = 5;
        x = 1.01;
        y = 7.01;
        break;
    case CLOUD:
        cosine = 8;
        x = -1;
        y = -1;
        animated = 9;
        break;
    default:
        break;
    }
    if ((type == LAVA || type == WATER) &&
        (face != TOP && face != BOTTOM)) {
        switch (i) {
        case 0:
            y += 0.98; break;
        case 1:
            break;
        case 2:
            x += 0.98; break;
        case 3:
            x += 0.98; y += 0.98; break;
        default: break;
        }
    } else {
        if (i == 1 || i == 2) {
            x += 0.98;
        }
        if (i == 2 || i == 3) {
            y += 0.98;
        }
    }
    verts.push_back(x / 16.0);
    verts.push_back(y / 16.0);
    verts.push_back(cosine);
    verts.push_back(animated);
}
