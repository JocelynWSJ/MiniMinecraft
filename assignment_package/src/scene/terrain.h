#pragma once
#include <QList>
#include "biome.h"
#include "chunk.h"
#include "rectangle.h"
#include "raindrop.h"
#include "lightening.h"
#include "snow.h"

class Terrain
{
    friend class MyGL;
private:
    // hashmap of all chucks
    std::map<int64_t, Chunk> m_chunks;
    std::map<int64_t, RainDrop> m_rain;
    std::map<int64_t, Snow> m_snow;
    std::map<int64_t, Lightening> m_lightening;

    // pass openGL context to chunks
    OpenGLContext* m_context;

public:
    // construct and initialize
    Terrain(OpenGLContext* m_context);

    // get the blocktype at a world-space position
    // return empty when no block is there
    BlockType getBlockAt(int x, int y, int z) const;
    // set the blocktype at a world-space position
    // when there is no chunk, do nothing
    void setBlockAt(int x, int y, int z, BlockType t);

    // find if there is a chunk at a world-space position
    bool hasChunk(int x, int z, int y = 128) const;
    // get the chunk at a world-space position, if no chunk, return nullptr
    Chunk* getChunk(int x, int z, int y = 128);

    // give a player world-space position, check if it's near boarder
    bool checkBooarder(int x, int z, Rect16 &result);
    // build basic terrain of a chunk
    void buildChunk(int x0, int z0);

    // ray cast from camera to terrain, removing or adding block by click
    void playerClick(glm::vec3 ori, glm::vec3 dir, bool add);

    // check if a given area is explored
    bool explored(const Rect64 &area) const;
    // check if a given domain is partly explored, ignore one area
    bool explored(const Domain &domain, const Rect64 &ignore) const;
    // use water to erode a location to a given height
    void waterErode(int x, int z, int restY);
    // place assets procedually, based on biomes
    void placeAssets(const Rect16 scope);

    // build weather
    void buildWeather(int x, int z, bool shouldCreate = false);
    // update the height
    void updateHeight(int x, int z, int h);
    // update weather
    void updateWeather(int x, int y, int z);
    // create cloud
    void createCloud(int x, int z);
    // if this pos can rain
    bool canRain(int x, int z) const;
    // if this pos can snow
    bool canSnow(int x, int z) const;

private:
    // given x and z coord, return a harsh int64 as the key of the map
    int64_t hash (int x, int z) const;
    // move a point to the origin of the chunk it lives in
    void moveToOrigin(int &x, int &z, int module = 16) const;

    // openGL create all chunks
    void create();
    // openGL destroy all chunks
    void destroy();

    // set up neigborhood for a chunk at given origin
    void setNeighbor(int x, int z);

    // build pending chunks with multi-thread
    void buildChunkThread();
    // build a group of 16 FBM chunks
    void buildFbmGroup(int baseX, int baseZ);
};
