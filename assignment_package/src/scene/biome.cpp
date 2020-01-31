#include "biome.h"
#include <iostream>

noise::fbmParams Biome::darkPars = noise::fbmParams();
noise::fbmParams Biome::desertPars = noise::fbmParams();
noise::fbmParams Biome::frozenPars = noise::fbmParams();
noise::fbmParams Biome::junglePars = noise::fbmParams();
noise::fbmParams Biome::moisturePars = noise::fbmParams();
noise::fbmParams Biome::temperaturePars = noise::fbmParams();

Biome::Biome(int _x, int _z): x(_x), z(_z) {
    moisture = noise::sealedFbm2D(x, z, moisturePars);
    moisture /= moisturePars.scaleY;
    moisture *= 1.33;
    moisture = noise::clamp(moisture, 0.f, 1.f);
    temperature = noise::sealedFbm2D(x, z, temperaturePars);
    temperature /= temperaturePars.scaleY;
    temperature *= 1.33;
    temperature = noise::clamp(temperature, 0.f, 1.f);
    if (moisture > 0.58 && temperature > 0.58) {
        type = JUNGLE;
    } else if (moisture > 0.45 && temperature > 0.5) {
        type = PLAIN;
    } else if (moisture > 0.55 && temperature <= 0.45) {
        type = FROZEN;
    } else if (moisture > 0.5 && temperature <= 0.5) {
        type = TUNDRA;
    } else if (moisture <= 0.4 && temperature <= 0.4) {
        type = DARK;
    } else if (moisture <= 0.5 && temperature <= 0.5) {
        type = MOUNTAIN;
    } else {
        type = DESERT;
    }
}

BiomeType Biome::getBiome() const {
    return type;
}

BiomeType Biome::getBiome(int &height) const {
    int darkHeight = 129 + noise::sealedFbm2D(x, z, darkPars);
    int desertHeight = 129 + noise::sealedFbm2D(x, z, desertPars);
    int frozenHeight = 129 + noise::sealedFbm2D(x, z, frozenPars);
    int jungleHeight = 129 + noise::sealedFbm2D(x, z, junglePars);
    height = noise::mix(noise::mix(darkHeight, frozenHeight, moisture),
                        noise::mix(desertHeight, jungleHeight, moisture),
                        temperature);
    return type;
}

void Biome::InitializeParams() {
    darkPars.exponent = 3;
    darkPars.scaleY = 60.f;
    darkPars.offsetY = 10;

    desertPars.exponent = 1;
    desertPars.scaleY = 4.f;

    frozenPars.exponent = 5;
    frozenPars.scaleY = 80.f;

    junglePars.exponent = 1;
    junglePars.scaleY = 10.f;
    junglePars.offsetY = -7;

    moisturePars.exponent = 1;
    moisturePars.scaleX = 0.002f;
    moisturePars.scaleY = 100.f;
    moisturePars.scaleZ = 0.002f;
    moisturePars.seed1 = 123.4f;

    temperaturePars.exponent = 1;
    temperaturePars.scaleX = 0.002f;
    temperaturePars.scaleY = 100.f;
    temperaturePars.scaleZ = 0.002f;
    temperaturePars.seed1 = 345.6f;
}
