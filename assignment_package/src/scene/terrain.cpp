#include "terrain.h"

// construct and initialize
Terrain::Terrain(OpenGLContext* context):
    m_chunks(), m_context(context)
{
    for (int x = 0; x < 64; x += 16) {
        for (int z = 0; z < 64; z += 16) {
            m_chunks.emplace(std::make_pair(hash(x, z), Chunk(m_context, glm::vec4(x, 0, z, 1))));
            buildWeather(x, z);
            setNeighbor(x, z);
            buildChunk(x, z);
        }
    }
}

// get the blocktype at a world-space position
// return empty when no block is there
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    int xo = x;
    int zo = z;
    moveToOrigin(xo, zo);
    // this chunk has not been created yet
    if (!m_chunks.count(hash(xo, zo))) {
        return EMPTY;
    }
    // get chunk and set block type
    return m_chunks.find(hash(xo, zo))->second.blockAt(x - xo, y, z - zo);
}

// set the blocktype at a world-space position
// when there is no chunk, do nothing
void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    int xo = x;
    int zo = z;
    moveToOrigin(xo, zo);
    // when there is no chunk, do nothing
    if (m_chunks.find(hash(xo, zo)) == m_chunks.end()) {
        return;
    }
    // get chunk and set block type
    m_chunks.find(hash(xo, zo))->second.blockAt(x - xo, y, z - zo) = t;
}

// find if there is a chunk at a world-space position
bool Terrain::hasChunk(int x, int z, int y) const {
    if (y < 0 || y > 255) {
        return false;
    }
    moveToOrigin(x, z);
    if (m_chunks.count(hash(x, z))) {
        return true;
    }
    return false;
}

// get the chunk at a world-space position, if no chunk, return nullptr
Chunk* Terrain::getChunk(int x, int z, int y) {
    if (y < 0 || y > 255) {
        return nullptr;
    }
    moveToOrigin(x, z);
    if (m_chunks.count(hash(x, z))) {
        return &(m_chunks.find(hash(x, z))->second);
    }
    return nullptr;
}

// give a player world-space position, check if it's near boarder
bool Terrain::checkBooarder(int x, int z, Rect16 &result) {
    for (int i = 0; i < 81; i++) {
        int xi = x + (i / 9) * 16 - 64;
        int zi = z + (i % 9) * 16 - 64;
        if (abs(xi - x) + abs(zi - z) > 80) {
            continue;
        }
        if (!hasChunk(xi, zi)) {
            moveToOrigin(xi, zi);
            m_chunks.emplace(std::make_pair(hash(xi, zi), Chunk(m_context, glm::vec4(xi, 0, zi, 1))));
            setNeighbor(xi, zi);
            result = Rect16(xi, zi);
            buildWeather(xi, zi, true);
            return true;
        }
    }
    return false;

}

// ray cast from camera to terrain, removing or adding block by click
void Terrain::playerClick(glm::vec3 ori, glm::vec3 dir, bool add) {
    dir = glm::normalize(dir);
    // length of the arm
    float length = 10.f;
    glm::ivec3 block = glm::ivec3((int)floorf(ori.x), (int)floorf(ori.y), (int)floorf(ori.z));
    glm::ivec3 backBlock = block;
    ori = glm::vec3(ori.x - block.x, ori.y - block.y, ori.z - block.z);
    glm::vec3 target = glm::vec3();
    while (length > 0) {
        for (int i = 0; i < 3; i++) {
            if (dir[i] != 0.f) {
                float t = 0.f;
                if (dir[i] > 0) {
                    t = (1 - ori[i]) / dir[i];
                    target[i] = 0.f;
                } else if (dir[i] < 0) {
                    t = -ori[i] / dir[i];
                    target[i] = 1.f;
                }
                int j = (i + 1) % 3;
                int k = (i + 2) % 3;
                float targetJ = ori[j] + dir[j] * t;
                float targetK = ori[k] + dir[k] * t;
                if (targetJ > 0 && targetJ < 1 && targetK > 0 && targetK < 1) {
                    target[j] = targetJ;
                    target[k] = targetK;
                    backBlock = block;
                    if (dir[i] > 0) {
                        block[i] += 1;
                    } else {
                        block[i] -= 1;
                    }
                    length -= t;
                    break;
                }
            }
        }
        ori = target;
        BlockType blockType = getBlockAt(block.x, block.y, block.z);
        if (blockType != EMPTY) {
            if (add && length < 8.f) {
                Chunk* chunk = getChunk(backBlock.x, backBlock.z, backBlock.y);
                if (chunk != nullptr) {
                    setBlockAt(backBlock.x, backBlock.y, backBlock.z, LAVA);
                    updateWeather(backBlock.x, backBlock.y, backBlock.z);
                    chunk->combinedCreate();
                }
            } else if (!add) {
                Chunk* chunk = getChunk(block.x, block.z, block.y);
                if (chunk != nullptr) {
                    setBlockAt(block.x, block.y, block.z, EMPTY);
                    updateWeather(backBlock.x, backBlock.y - 1, backBlock.z);
                    chunk->combinedCreate();
                }
            }
            break;
        }
    }
}

// check if a given area is explored
bool Terrain::explored(const Rect64 &area) const {
    for (int i = 0; i < 64; i += 16) {
        for (int j = 0; j < 64; j += 16) {
            if (hasChunk(area.xmin + i, area.zmin + j)) {
                return true;
            }
        }
    }
    return false;
}

// check if a given domain is partly explored, ignore one area
bool Terrain::explored(const Domain &domain, const Rect64 &ignore) const {
    for (int i = 0; i < domain.rects.size(); i++) {
            if (!(domain.rects[i] == ignore) && explored(domain.rects[i])) return true;
        }
        return false;
}

void Terrain::buildWeather(int x, int z, bool shouldCreate) {
    moveToOrigin(x, z);
    if (canRain(x + 8, z + 8)) {
        RainDrop raindrop(m_context,
             glm::vec4(x, 128, z, 1),
             glm::vec4(0.28, 0.44, 0.76, 0.8));
        m_rain.emplace(std::make_pair(hash(x, z), raindrop));
        if (shouldCreate) {
            m_rain.find(hash(x, z))->second.create();
        }
    }
    else if (canSnow(x + 8, z + 8)) {
        Snow snow(m_context,
             glm::vec4(x, 128, z, 1));
        m_snow.emplace(std::make_pair(hash(x, z), snow));
        if (shouldCreate) {
            m_snow.find(hash(x, z))->second.create();
        }
    }
    if (z >= 128) {
        x = -32;
        z = 256;
        Lightening lightening(m_context, glm::vec4(x, 0, z, 1));
        m_lightening.emplace(std::make_pair(hash(x, z), lightening));
        if (shouldCreate) {
            m_lightening.find(hash(x, z))->second.create();
        }
    }
}

void Terrain::updateHeight(int x, int z, int h) {
    int xpos = x;
    int zpos = z;
    moveToOrigin(x, z);
    if (canRain(x + 8, z + 8)) {
        int64_t first = hash(x, z);
        if (m_rain.find(first) != m_rain.end()) {
            m_rain.find(first)->second.setHeight(xpos - x, zpos - z, h);
        }
    }
}

void Terrain::updateWeather(int x, int y, int z) {
    int xpos = x;
    int zpos = z;
    moveToOrigin(x, z);
    if (canRain(x + 8, z + 8)) {
        updateHeight(xpos, zpos, y);
        m_rain.find(hash(x, z))->second.destroy();
        m_rain.find(hash(x, z))->second.create();
    }
}

void Terrain::createCloud(int px, int pz) {
    moveToOrigin(px, pz);
    if (pz % 64 != 0 || px % 64 != 0) {
        return;
    }
    float seed = noise::rand2D((float)(px % 1024), (float)(pz % 1024),
                               123.4f, 345.6f, 456.7f) * 111.f;
    //float rand = noise::rand1D(seed);
    //if (rand < 0.6f) {
        //return;
    //}
    int xori = (int)(noise::rand1D(seed + 12.3) * 3) + px;
    int xdim = (int)(noise::rand1D(seed + 123.4) * 9) + 4;
    int zori = (int)(noise::rand1D(seed + 23.4) * 3) + pz;
    int zdim = (int)(noise::rand1D(seed + 2314.5) * 9) + 4;
    //int height = (int)(noise::rand1D(seed + 234.5) * 20) + 160;
    int height = 180;
    for (int dx = 0; dx <= xdim; dx++) {
        for (int dz = 0; dz <= zdim; dz++) {
            setBlockAt(xori + dx, height, zori + dz, CLOUD);
        }
    }
}

bool Terrain::canRain(int x, int z) const {
    Biome biome(x, z);
    BiomeType biomeType = biome.getBiome();
    if (biomeType == JUNGLE) {
        return true;
    } else {
        return false;
    }
}

bool Terrain::canSnow(int x, int z) const {
    Biome biome(x, z);
    BiomeType biomeType = biome.getBiome();
    if (biomeType == FROZEN) {
        return true;
    } else {
        return false;
    }
}

// given x and z coord, return a harsh int64 as the key of the map
int64_t Terrain::hash(int x, int z) const{
    int64_t ix = int64_t(x);
    int64_t iz = int64_t(z);
    return (ix << 32) + iz;
}

// move a point to the origin of the chunk it lives in
void Terrain::moveToOrigin(int &x, int &z, int module) const {
    if (module <= 0) {
        return;
    }

    if (x % module == 0) {
    } else if (x < 0) {
        x -= x % module + module;
    } else {
        x -= x % module;
    }

    if (z % module == 0) {
    } else if (z < 0) {
        z -= z % module + module;
    } else {
        z -= z % module;
    }
}

// openGL create all chunks
void Terrain::create() {
    for (auto it = m_chunks.begin(); it != m_chunks.end(); it++) {
        (it->second).create();
    }
    for (auto it = m_rain.begin(); it != m_rain.end(); it++) {
        (it->second).create();
    }
    for (auto it = m_snow.begin(); it != m_snow.end(); it++) {
        (it->second).create();
    }
    for (auto it = m_lightening.begin(); it != m_lightening.end(); it++) {
        (it->second).create();
    }
}

// openGL destroy all chunks
void Terrain::destroy() {
    for (auto it = m_chunks.begin(); it != m_chunks.end(); it++) {
        (it->second).destroy();
    }
    for (auto it = m_rain.begin(); it != m_rain.end(); it++) {
        (it->second).destroy();
    }
    for (auto it = m_snow.begin(); it != m_snow.end(); it++) {
        (it->second).destroy();
    }
    for (auto it = m_lightening.begin(); it != m_lightening.end(); it++) {
        (it->second).destroy();
    }
}

// set up neigborhood for a chunk at given origin
void Terrain::setNeighbor(int x, int z) {
    if (!hasChunk(x, z)) {
        return;
    }
    if (hasChunk(x - 16, z)) {
        m_chunks.find(hash(x, z))->second.left =
                &m_chunks.find(hash(x - 16, z))->second;
        m_chunks.find(hash(x - 16, z))->second.right =
                &m_chunks.find(hash(x, z))->second;
    }
    if (hasChunk(x + 16, z)) {
        m_chunks.find(hash(x, z))->second.right =
                &m_chunks.find(hash(x + 16, z))->second;
        m_chunks.find(hash(x + 16, z))->second.left =
                &m_chunks.find(hash(x, z))->second;
    }
    if (hasChunk(x, z - 16)) {
        m_chunks.find(hash(x, z))->second.back =
                &m_chunks.find(hash(x, z - 16))->second;
        m_chunks.find(hash(x, z - 16))->second.front =
                &m_chunks.find(hash(x, z))->second;
    }
    if (hasChunk(x, z + 16)) {
        m_chunks.find(hash(x, z))->second.front =
                &m_chunks.find(hash(x, z + 16))->second;
        m_chunks.find(hash(x, z + 16))->second.back =
                &m_chunks.find(hash(x, z))->second;
    }
}
