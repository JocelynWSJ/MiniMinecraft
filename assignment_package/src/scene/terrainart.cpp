#include "terrain.h"

// build basic terrain of a chunk
void Terrain::buildChunk(int x0, int z0) {
    Biome::InitializeParams();
    moveToOrigin(x0, z0);
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int xi = x + x0;
            int zi = z + z0;
            int top = 0;
            Biome biome(xi, zi);
            BiomeType biomeType = biome.getBiome(top);
            // lake feature
            if (top <= 128) { top -= 1; }
            // sand dune feature
            if (biomeType == DESERT && top > 134) {
                top = 134 + (int)((float)(top - 134) * 0.3f);
            }
            int bounce = top;
            // place blocks
            for (int y = 0; y < 256; y++) {
                if (y <= 128 && y < top) {
                    setBlockAt(xi, y, zi, STONE);
                } else if (y <= 128 && y == top) {
                    setBlockAt(xi, y, zi, BEDROCK);
                } else if (y <= 128) {
                    switch (biomeType) {
                    case FROZEN:
                    case TUNDRA:
                    case DARK:
                    case MOUNTAIN:
                        setBlockAt(xi, y, zi, ICE);
                        break;
                    default:
                        setBlockAt(xi, y, zi, WATER);
                        bounce = -1;
                        break;
                    }
                } else if (y < top) {
                    switch (biomeType) {
                    case PLAIN:
                    case TUNDRA:
                    case FROZEN:
                        setBlockAt(xi, y, zi, DIRT);
                        break;
                    case DARK:
                        setBlockAt(xi, y, zi, EVIL);
                        break;
                    case DESERT:
                        setBlockAt(xi, y, zi, SAND);
                        break;
                    case JUNGLE:
                        setBlockAt(xi, y, zi, LEAFMOLD);
                        break;
                    case MOUNTAIN:
                        setBlockAt(xi, y, zi, STONE);
                        break;
                    default:
                        break;
                    }
                } else if (y == top) {
                    switch (biomeType) {
                    case PLAIN:
                        setBlockAt(xi, y, zi, GRASS);
                        break;
                    case DARK:
                        setBlockAt(xi, y, zi, EVIL);
                        break;
                    case DESERT:
                        setBlockAt(xi, y, zi, SAND);
                        break;
                    case FROZEN:
                        setBlockAt(xi, y, zi, SNOW);
                        break;
                    case JUNGLE:
                        setBlockAt(xi, y, zi, LEAFMOLD);
                        break;
                    case TUNDRA:
                        setBlockAt(xi, y, zi, FROZEDIRT);
                        break;
                    case MOUNTAIN:
                        setBlockAt(xi, y, zi, STONE);
                        break;
                    default:
                        break;
                    }
                } else {
                    setBlockAt(xi, y, zi, EMPTY);
                }
            }
            updateHeight(xi, zi, bounce);
        }
    }
    createCloud(x0, z0);
}

// use water to erode a location to a given height
void Terrain::waterErode(int x, int z, int restY) {
    Biome biome(x, z);
    BiomeType biomeType = biome.getBiome();
    bool hasWater = false;
    for (int y = 255; y > restY; y--) {
        if (y > 128) {
            setBlockAt(x, y, z, EMPTY);
        } else {
            hasWater = true;
            switch (biomeType) {
            case FROZEN:
            case TUNDRA:
            case DARK:
            case MOUNTAIN:
                setBlockAt(x, y, z, ICE);
                break;
            default:
                setBlockAt(x, y, z, WATER);
                break;
            }
        }
    }
    if (hasWater) {
        updateHeight(x, z, -1);
    }
    BlockType restType = getBlockAt(x, restY, z);
    if (restType != EMPTY && restType != WATER &&
        restType != LAVA && restType != ICE) {
        if (restY >= 128) {
            switch (biomeType) {
            case PLAIN:
                setBlockAt(x, restY, z, GRASS);
                break;
            case DARK:
                setBlockAt(x, restY, z, EVIL);
                break;
            case DESERT:
                setBlockAt(x, restY, z, SAND);
                break;
            case FROZEN:
                setBlockAt(x, restY, z, SNOW);
                break;
            case JUNGLE:
                setBlockAt(x, restY, z, LEAFMOLD);
                break;
            case TUNDRA:
                setBlockAt(x, restY, z, FROZEDIRT);
                break;
            case MOUNTAIN:
                setBlockAt(x, restY, z, STONE);
                break;
            default:
                break;
            }
        } else {
            setBlockAt(x, restY, z, BEDROCK);
        }
        updateHeight(x, z, restY);
    }
}

// place assets procedually, based on biomes
void Terrain::placeAssets(const Rect16 scope) {
    noise::fbmParams glacierParams;
    glacierParams.exponent = 3;
    glacierParams.seed1 = 432.1f;
    noise::fbmParams goldParams;
    goldParams.exponent = 3;
    goldParams.scaleX = 0.1f;
    goldParams.scaleZ = 0.1f;
    goldParams.seed1 = 135.7;
    noise::fbmParams coalParams;
    coalParams.exponent = 3;
    coalParams.scaleX = 0.1f;
    coalParams.scaleZ = 0.1f;
    coalParams.seed1 = 357.9;
    noise::fbmParams rubyParams;
    rubyParams.exponent = 3;
    rubyParams.scaleX = 0.1f;
    rubyParams.scaleZ = 0.1f;
    rubyParams.seed1 = 579.1;
    for (int x = scope.xmin; x <= scope.xmax; x++) {
        for (int z = scope.zmin; z <= scope.zmax; z++) {
            // find top height
            int top = 255;
            for (int y = 255; y >= 100; y--) {
                if (Chunk::isCollidable(getBlockAt(x, y, z))) {
                    top = y;
                    break;
                }
            }
            // create random value
            float seed = noise::rand2D((float)(x % 1024), (float)(z % 1024),
                                       123.4f, 345.6f, 456.7f) * 111.f;
            float rand = noise::rand1D(seed);
            // compute biome type
            Biome biome(x, z);
            BiomeType biomeType = biome.getBiome();
            // place different assests
            if (biomeType == PLAIN) {
                // grass and flower
                if (top > 128 && getBlockAt(x, top, z) == GRASS) {
                    if (rand > 0.95) {
                        setBlockAt(x, top + 1, z, MUSHROOM);
                    } else if (rand > 0.9) {
                        setBlockAt(x, top + 1, z, REDFLOWER);
                    } else if (rand > 0.6) {
                        setBlockAt(x, top + 1, z, CROSSGRASS);
                    }
                }
            } else if (biomeType == DARK) {
                // lava
                if (top > 130) {
                    if (rand > (float)(165 - top) / 30.f && rand > 0.3f) {
                        int end = top - (int)((float)(top - 135) * rand);
                        for (int y = top; y >= end; y--) {
                            setBlockAt(x, y, z, LAVA);
                        }
                    }
                }
            } else if (biomeType == DESERT) {
                // red rock
                int bottom = 135;
                if (top > bottom) {
                    int orange = top + 1;
                    int red = top + 3 + (int)((float)(top - bottom) * 0.3f);
                    for (int y = top + 1; y <= red; y++) {
                        if (y <= orange) {
                            setBlockAt(x, y, z, ORANGEROCK);
                        } else {
                            setBlockAt(x, y, z, REDROCK);
                        }
                    }
                }
            } else if (biomeType == FROZEN) {
                // glacier
                if (top > 120 && top < 140) {
                    if (noise::sealedFbm2D(x, z, glacierParams) /
                        glacierParams.scaleY > 0.25f * (float)(top - 130) / 10.f) {
                        int end = top + 1 + (int)((float)(140 - top) * 0.15f);
                        for (int y = end; y > top; y--) {
                            setBlockAt(x, y, z, ICE);
                        }
                    }
                }
            } else if (biomeType == JUNGLE) {
                // tree
                if (top > 128 && top < 140 && rand > 0.92 &&
                    x > scope.xmin + 1 && x < scope.xmax - 1 &&
                    z > scope.zmin + 1 && z < scope.zmax - 1 &&
                    getBlockAt(x, top, z) == LEAFMOLD) {
                    int height = 2 + (int)(5 * noise::rand1D(seed + 12.3f));
                    BlockType leaf = noise::rand1D(seed + 23.4f) > 0.3 ?
                                     BlockType::LEAF : BlockType::LEAFMOLD;
                    for (int y = top + 1; y < top + height; y++) {
                        setBlockAt(x, y, z, WOOD);
                    }
                    int y  = top + height;
                    for (int xi = x - 1; xi <= x + 1; xi++) {
                        for (int zi = z - 1; zi <= z + 1; zi++) {
                            setBlockAt(xi, y, zi, leaf);
                        }
                    }
                    y++;
                    for (int xi = x - 2; xi <= x + 2; xi++) {
                        for (int zi = z - 2; zi <= z + 2; zi++) {
                            setBlockAt(xi, y, zi, leaf);
                        }
                    }
                    y++;
                    for (int xi = x - 1; xi <= x + 1; xi++) {
                        for (int zi = z - 1; zi <= z + 1; zi++) {
                            setBlockAt(xi, y, zi, leaf);
                        }
                    }
                    y++;
                    int x0 = noise::rand1D(seed + 34.5f) > 0.5f ? x - 1 : x;
                    int z0 = noise::rand1D(seed + 45.6f) > 0.5f ? z - 1 : z;
                    setBlockAt(x0, y, z0, leaf);
                    setBlockAt(x0 + 1, y, z0, leaf);
                    setBlockAt(x0, y, z0 + 1, leaf);
                    setBlockAt(x0 + 1, y, z0 + 1, leaf);
                }
            } else if (biomeType == TUNDRA) {
                // hardy plant
                if (top > 128 && getBlockAt(x, top, z) == FROZEDIRT) {
                    if (rand > 0.98) {
                        setBlockAt(x, top + 1, z, BUSH);
                    } else if (rand > 0.96) {
                        setBlockAt(x, top + 1, z, DEADBRANCH);
                    } else if (rand > 0.9) {
                        setBlockAt(x, top + 1, z, GREYMUSHROOM);
                    }
                }
            } else if (biomeType == MOUNTAIN) {
                // ore
                int bottom = 130;
                if (top > bottom) {
                    if (noise::sealedFbm2D(x, z, rubyParams) /
                        rubyParams.scaleY > 0.2f) {
                        int low = bottom + 13 + (int)((float)(top - bottom) * 0.05f);
                        int high = bottom + 14 + (int)((float)(top - bottom) * 0.1f);
                        for (int y = low; y <= high && y <= top; y++) {
                            setBlockAt(x, y, z, RUBY);
                        }
                    }
                    if (noise::sealedFbm2D(x, z, goldParams) /
                        goldParams.scaleY > 0.15f) {
                        int low = bottom + 8 + (int)((float)(top - bottom) * 0.05f);
                        int high = bottom + 9 + (int)((float)(top - bottom) * 0.1f);
                        for (int y = low; y <= high && y <= top; y++) {
                            setBlockAt(x, y, z, GOLD);
                        }
                    }
                    if (noise::sealedFbm2D(x, z, coalParams) /
                        coalParams.scaleY > 0.08f) {
                        int low = bottom + 2 + (int)((float)(top - bottom) * 0.05f);
                        int high = bottom + 5 + (int)((float)(top - bottom) * 0.1f);
                        for (int y = low; y <= high && y <= top; y++) {
                            setBlockAt(x, y, z, COAL);
                        }
                    }
                }
            }
        }
    }
}
