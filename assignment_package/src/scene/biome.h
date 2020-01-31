#ifndef BIOME_H
#define BIOME_H
#include "noise.h"

enum BiomeType : unsigned char
{
    PLAIN, DARK, DESERT, FROZEN, JUNGLE, TUNDRA, MOUNTAIN
};

class Biome
{
private:
    int x;
    int z;
    float moisture;
    float temperature;
    BiomeType type;
private:
    static noise::fbmParams darkPars;
    static noise::fbmParams desertPars;
    static noise::fbmParams frozenPars;
    static noise::fbmParams junglePars;
    static noise::fbmParams moisturePars;
    static noise::fbmParams temperaturePars;
public:
    Biome(int _x, int _z);
    BiomeType getBiome() const;
    BiomeType getBiome(int &height) const;
public:
    static void InitializeParams();
};

#endif // BIOME_H
