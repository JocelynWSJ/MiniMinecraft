#ifndef NOISE_H
#define NOISE_H

#include <cmath>

namespace noise {

struct fbmParams {
    float persistence = 0.5f;
    int octaves = 8;
    int exponent = 4;
    float scaleX = 0.02f;
    float scaleY = 30.0f;
    float scaleZ = 0.02f;
    int offsetX = 0;
    int offsetY = 0;
    int offsetZ = 0;
    float seed1 = 12.9898f;
    float seed2 = 4.1414f;
    float seed3 = 43858.5453f;
};

// get fractional part of a float
static inline float fractf(float x) {
    return x - floorf(x);
}

// get power of a float
static inline float powf(float base, int exp) {
    float result = 1.0f;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

// clamp a float between 2 numbers
static inline float clamp(float x, float min, float max) {
    return x < min ? min : (x > max ? max : x);
}

// blend two floats based on 0<t<1
static inline float mix(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

// cubic blending of two floats
static inline float mixCubic(float a, float b, float t) {
    t = t * t * (3.0f - 2.0f * t);
    return mix(a, b, t);
}

// get the square of length of a 2D vector
static inline float sqrDist2D(float dx, float dy) {
    return dx * dx + dy * dy;
}

// dot product of two 2D vectors
static inline float dot2D(float ax, float ay, float bx, float by) {
    return ax * bx + ay * by;
}

// get the area of triangle defined by two 2D vectors
static inline float area2D(float ax, float ay, float bx, float by) {
    return std::abs(ax * by - bx * ay) / 2.f;
}

// get random value between (0,1) based on a 2D point
static inline float rand2D(float x, float y, float s1, float s2, float s3) {
    return fractf(sinf(dot2D(x, y, s1, s2)) * s3);
}

// get a random value between (0,1)
static inline float rand1D(float seed) {
    return rand2D(seed, seed + 123.4f, seed + 345.6f, seed + 678.9f, seed + 987.6f);
}

// get a random value between (a,b)
static inline float randRange(float a, float b, float seed) {
    return a + (b - a) * rand1D(seed);
}

// get interpolaed random value between (0,1) of a 2D point
static inline float interpRand2D(float x, float y, float s1, float s2, float s3) {
    float intX = floorf(x);
    float fractX = fractf(x);
    float intY = floorf(y);
    float fractY = fractf(y);

    float v1 = rand2D(intX, intY, s1, s2, s3);
    float v2 = rand2D(intX + 1.0f, intY, s1, s2, s3);
    float v3 = rand2D(intX, intY + 1.0f, s1, s2, s3);
    float v4 = rand2D(intX + 1.0f, intY + 1.0f, s1, s2, s3);

    float i1 = mixCubic(v1, v2, fractX);
    float i2 = mixCubic(v3, v4, fractX);
    return mixCubic(i1, i2, fractY);
}

// get fractal Brownian movement value between (0,1) of a 2D point
static inline float fbm2D(float x, float y, float persistence, int octaves,
                          float s1, float s2, float s3) {
    float total = 0.0f;
    float freq = 1.0f;
    float amp = 1.0f;
    for (int i = 0; i < octaves; i++) {
        freq *= 2.0f;
        amp *= persistence;
        total += interpRand2D(x * freq, y * freq, s1, s2, s3) * amp;
    }
    return total;
}

// get fbm value of a 2D point using sealed parameters
static inline int sealedFbm2D(int x, int z, const fbmParams &p) {
    return p.offsetY + (int)(p.scaleY *
           powf(fbm2D((float)(x + p.offsetX) * p.scaleX,
                      (float)(z + p.offsetZ) * p.scaleZ,
                      p.persistence, p.octaves,
                      p.seed1, p.seed2, p.seed3),
                p.exponent));
}

} // namespace noise

#endif // NOISE_H
