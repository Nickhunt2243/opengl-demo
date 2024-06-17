#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

#include "noise.hpp"

namespace Craft
{
    /// Code adopted from the Java code here: https://mrl.cs.nyu.edu/~perlin/noise/
    /// Additional information on the improved Noise algorithm:
    /// chrome-extension://efaidnbmnnnibpcajpcglclefindmkaj/https://mrl.cs.nyu.edu/~perlin/paper445.pdf
    Noise::Noise(unsigned int seed) {
        p.resize(512);
        // Initialize the permutation vector with values 0 to 255
        std::vector<int> permutation(256);
        std::iota(permutation.begin(), permutation.end(), 0);

        // Shuffle the permutation vector using the provided seed
        std::shuffle(permutation.begin(), permutation.end(), std::default_random_engine(seed));

        // Duplicate the permutation vector
        for (int i = 0; i < 256; ++i) {
            p[i] = permutation[i];
            p[256 + i] = permutation[i];
        }
    }
    float Noise::fractalNoise(
            float x, float y, float z,
            int octaves,
            float persistence,
            float amplitude,
            float frequency
        ) {
        float total = 0;
        float maxValue = 0;  // Used for normalizing result to [0, octaves * amplitude]

        for (int i = 0; i < octaves; i++) {
            total += noise(x * frequency, y * frequency, z * frequency) * amplitude;

            maxValue += amplitude;

            amplitude *= persistence;
            frequency *= 2;
        }

        return total / maxValue;
    }
    float Noise::noise(float x, float y, float z) {
        int X = (int) floor(x) & 255,                  // FIND UNIT CUBE THAT
            Y = (int) floor(y) & 255,                  // CONTAINS POINT.
            Z = (int) floor(z) & 255;
        x -= floor(x);                                 // FIND RELATIVE X,Y,Z
        y -= floor(y);                                 // OF POINT IN CUBE.
        z -= floor(z);
        float u = fade(x),                             // COMPUTE FADE CURVES
              v = fade(y),                             // FOR EACH OF X,Y,Z.
              w = fade(z);
        int A = p[X  ]+Y,
            AA = p[A]+Z,
            AB = p[A+1]+Z,                             // HASH COORDINATES OF
            B = p[X+1]+Y,                              // THE 8 CUBE CORNERS,
            BA = p[B]+Z,
            BB = p[B+1]+Z;

        return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
                                    grad(p[BA  ], x-1, y  , z   )), // BLENDED
                            lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
                                 grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
                    lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
                                 grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
                         lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                              grad(p[BB+1], x-1, y-1, z-1 ))));
    }
    float Noise::fade(float t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    float Noise::lerp(float t, float a, float b)
    {
        return a + t * (b - a);
    }
    float Noise::grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
        double u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
        v = h<4 ? y : h==12||h==14 ? x : z;
        return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
    }
}