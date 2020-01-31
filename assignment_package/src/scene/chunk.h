#ifndef CHUNK_H
#define CHUNK_H
#include "drawable.h"
#include "la.h"
#include "smartpointerhelp.h"

enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, LAVA, WATER, SNOW, BEDROCK,
    WOOD, LEAF, ICE, REDFLOWER, CROSSGRASS, MUSHROOM, LAKEBOTTOM,
    SAND, EVIL, LEAFMOLD, FROZEDIRT, GREYMUSHROOM, BUSH, DEADBRANCH,
    GOLD, COAL, RUBY, YELLOWROCK, ORANGEROCK, REDROCK, CLOUD
};

enum FaceType : unsigned char
{
    LEFT, RIGHT, FRONT, BACK, TOP, BOTTOM
};

class ChunkCreateInfo
{
public:
    std::vector<GLuint> idx0;
    std::vector<GLuint> idx1;
    std::vector<float> opaque;
    std::vector<float> transparency;
};

class Chunk : public Drawable
{
    friend class Terrain;

private:
    // A 1D list of blocks
    std::vector<BlockType> m_blocks;
    // word position of the origin
    glm::vec4 m_originPos;
    // the neighbors of the chunks
    Chunk* left;
    Chunk* right;
    Chunk* front;
    Chunk* back;

public:
    Chunk(OpenGLContext* context) :
        Drawable(context),
        m_blocks(std::vector<BlockType>(16 * 256 * 16)),
        left(nullptr), right(nullptr), front(nullptr), back(nullptr) {}
    Chunk(OpenGLContext* context, glm::vec4 pos) :
        Drawable(context),
        m_blocks(std::vector<BlockType>(16 * 256 * 16)),
        m_originPos(pos),
        left(nullptr), right(nullptr), front(nullptr), back(nullptr) {}
    virtual ~Chunk() {}
    // openGL create
    void create() override;
    void create(const ChunkCreateInfo *info);
    // populate and create this and all neighbors
    void combinedCreate();
    // populate chunk create info
    void populateInfo(ChunkCreateInfo *info) const;
    // populate chunk create info of neighbors
    void populateNeighbor(ChunkCreateInfo *li, ChunkCreateInfo *ri,
                          ChunkCreateInfo *fi, ChunkCreateInfo *bi) const;
    // recreate this chunk
    void updateSelf(const ChunkCreateInfo *info);
    // recreate all neighbors
    void updateNeighbor(const ChunkCreateInfo *li, const ChunkCreateInfo *ri,
                        const ChunkCreateInfo *fi, const ChunkCreateInfo *bi);
    // get the blocktype located at that position in this chunk
    BlockType blockAt(int x, int y, int z) const;
    // set the blocktype located at that position in this Chunk
    BlockType& blockAt(int x, int y, int z);
public:
    static bool isOpaqueType(BlockType type);
    static bool isCollidable(BlockType type);
    static bool isCrossType(BlockType type);
private:
    // set up vbo for all non-empty cubes in this chunk
    void createCubes(std::vector<float>& opaque,
                     std::vector<float>& transparency,
                     std::vector<GLuint>& idx0,
                     std::vector<GLuint>& idx1) const;
    // is empty or transparent
    bool isBlockOpaque(int i, int j, int k) const;
    // return the index located at that position in this chunk
    int getIndex(int x, int y, int z) const;
    // determine whether a face should be painted
    bool shouldPaint(int i, int j, int k, FaceType face) const;
    // visit neighboring blocks and set up vbo for a single block
    void visitBlocks(int x, int y, int z,
                     std::vector<float>& verts,
                     std::vector<float>& transparency,
                     std::vector<GLuint>& idx0,
                     std::vector<GLuint>& idx1) const;
    // add face for a block
    void addFace(std::vector<glm::vec4> vertices, BlockType type,
                 std::vector<float>& verts,
                 std::vector<float>& transparency,
                 std::vector<GLuint>& idx0,
                 std::vector<GLuint>& idx1,
                 FaceType face) const;
    // add uv for a block
    void addUV(std::vector<float>& verts,
               BlockType type,
               FaceType face,
               int i) const;
};

#endif // CHUNK_H
