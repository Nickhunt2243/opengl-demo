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
    Noise::Noise(uint32_t seed) {
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
    __m256 Noise::__m256_fractalNoise_ps(
            __m256 x,
            __m256 y,
            __m256 z,
            int octaves,
            float persistence,
            float amplitude,
            float frequency
    ) {
        __m256 total = _mm256_set1_ps(0.0f);
        __m256 maxValue = _mm256_set1_ps(0.0f);
        __m256 amplitude_ps = _mm256_set1_ps(amplitude);
        __m256 frequency_ps = _mm256_set1_ps(frequency);

        for (int i = 0; i < octaves; i++) {
            // Compute noise with the current frequency and amplitude
            __m256 noiseResult = _mm256_noise_ps(_mm256_mul_ps(x, frequency_ps),
                                                 _mm256_mul_ps(y, frequency_ps),
                                                 _mm256_mul_ps(z, frequency_ps)
            );
            total = _mm256_fmadd_ps(
                        noiseResult,
                        amplitude_ps,
                        total
            );

            // Update maxValue and the amplitude for the next octave
            maxValue = _mm256_add_ps(maxValue, amplitude_ps);

            // Update amplitude and frequency for the next octave
            amplitude_ps = _mm256_mul_ps(amplitude_ps, _mm256_set1_ps(persistence));
            frequency_ps = _mm256_mul_ps(frequency_ps, _mm256_set1_ps(2.0f));
        }

        // Normalize the result
        return _mm256_div_ps(total, maxValue);
    }

// SIMD version of the noise function
    __m256 Noise::_mm256_noise_ps(__m256 x, __m256 y, __m256 z)
    {

        __m256i X = _mm256_and_epi32(_mm256_cvttps_epi32(_mm256_floor_ps(x)), _mm256_set1_epi32(255));
        __m256i Y = _mm256_and_epi32(_mm256_cvttps_epi32(_mm256_floor_ps(y)), _mm256_set1_epi32(255));
        __m256i Z = _mm256_and_epi32(_mm256_cvttps_epi32(_mm256_floor_ps(z)), _mm256_set1_epi32(255));

        x = _mm256_sub_ps(x, _mm256_floor_ps(x));
        y = _mm256_sub_ps(y, _mm256_floor_ps(y));
        z = _mm256_sub_ps(z, _mm256_floor_ps(z));

        __m256 u = _mm256_fade_ps(x);
        __m256 v = _mm256_fade_ps(y);
        __m256 w = _mm256_fade_ps(z);

        __m256i c1 = _mm256_set1_epi32(1);
        __m256i A = _mm256_add_epi32(_mm256_i32gather_epi32(p.data(), X, sizeof(int)), Y);
        __m256i B = _mm256_add_epi32(_mm256_i32gather_epi32(p.data(), _mm256_add_epi32(X, c1), sizeof(int)), Y);
        __m256i A1 = _mm256_add_epi32(A, c1);
        __m256i B1 = _mm256_add_epi32(B, c1);

        __m256i AA_indices = _mm256_add_epi32(_mm256_i32gather_epi32(p.data(), A, sizeof(int)), Z);
        __m256i AB_indices = _mm256_add_epi32(_mm256_i32gather_epi32(p.data(), A1, sizeof(int)), Z);
        __m256i BA_indices = _mm256_add_epi32(_mm256_i32gather_epi32(p.data(), B, sizeof(int)), Z);
        __m256i BB_indices = _mm256_add_epi32(_mm256_i32gather_epi32(p.data(), B1, sizeof(int)), Z);

        __m256i AA_indices_1 = _mm256_add_epi32(AA_indices, c1);
        __m256i AB_indices_1 = _mm256_add_epi32(AB_indices, c1);
        __m256i BA_indices_1 = _mm256_add_epi32(BA_indices, c1);
        __m256i BB_indices_1 = _mm256_add_epi32(BB_indices, c1);

        __m256i AA_values = _mm256_i32gather_epi32(p.data(), AA_indices, sizeof(int));
        __m256i AB_values = _mm256_i32gather_epi32(p.data(), AB_indices, sizeof(int));
        __m256i BA_values = _mm256_i32gather_epi32(p.data(), BA_indices, sizeof(int));
        __m256i BB_values = _mm256_i32gather_epi32(p.data(), BB_indices, sizeof(int));

        __m256i AA_values_1 = _mm256_i32gather_epi32(p.data(), AA_indices_1, sizeof(int));
        __m256i AB_values_1 = _mm256_i32gather_epi32(p.data(), AB_indices_1, sizeof(int));
        __m256i BA_values_1 = _mm256_i32gather_epi32(p.data(), BA_indices_1, sizeof(int));
        __m256i BB_values_1 = _mm256_i32gather_epi32(p.data(), BB_indices_1, sizeof(int));

        __m256 a = _mm256_grad_ps(AA_values, x , y, z);

        // Compute the gradients and perform the necessary lerp operations
        return _mm256_lerp_ps(
                w,
                _mm256_lerp_ps(
                        v,
                        _mm256_lerp_ps(u, _mm256_grad_ps(AA_values, x, y, z),
                                       _mm256_grad_ps(BA_values, _mm256_sub_ps(x, _mm256_set1_ps(1.0f)), y, z)),
                        _mm256_lerp_ps(u, _mm256_grad_ps(AB_values, x, _mm256_sub_ps(y, _mm256_set1_ps(1.0f)), z),
                                       _mm256_grad_ps(BB_values, _mm256_sub_ps(x, _mm256_set1_ps(1.0f)), _mm256_sub_ps(y, _mm256_set1_ps(1.0f)), z))),
                _mm256_lerp_ps(
                        v,
                        _mm256_lerp_ps(u, _mm256_grad_ps(AA_values_1, x, y, _mm256_sub_ps(z, _mm256_set1_ps(1.0f))),
                                       _mm256_grad_ps(BA_values_1, _mm256_sub_ps(x, _mm256_set1_ps(1.0f)), y, _mm256_sub_ps(z, _mm256_set1_ps(1.0f)))),
                        _mm256_lerp_ps(u, _mm256_grad_ps(AB_values_1, x, _mm256_sub_ps(y, _mm256_set1_ps(1.0f)), _mm256_sub_ps(z, _mm256_set1_ps(1.0f))),
                                       _mm256_grad_ps(BB_values_1, _mm256_sub_ps(x, _mm256_set1_ps(1.0f)), _mm256_sub_ps(y, _mm256_set1_ps(1.0f)), _mm256_sub_ps(z, _mm256_set1_ps(1.0f))))));
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
        int A = p[X]+Y,
            AA = p[A]+Z,
            AB = p[A+1]+Z,                             // HASH COORDINATES OF
            B = p[X+1]+Y,                              // THE 8 CUBE CORNERS,
            BA = p[B]+Z,
            BB = p[B+1]+Z;

        return lerp(w, lerp(v, lerp(u, grad(p[AA], x  , y  , z   ),  // AND ADD
                                    grad(p[BA], x-1, y  , z   )), // BLENDED
                            lerp(u, grad(p[AB], x  , y-1, z   ),  // RESULTS
                                 grad(p[BB], x-1, y-1, z   ))),// FROM  8
                    lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
                                 grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
                         lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                              grad(p[BB+1], x-1, y-1, z-1 ))));
    }
    float Noise::fade(float t)
    {
        return (t * (t * (t * ((t * ((t * 6) - 15)) + 10))));
    }
    float Noise::lerp(float t, float a, float b)
    {
        return a + t * (b - a);
    }
    float Noise::grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
        float u = h<8 ? x : y;                 // INTO 12 GRADIENT DIRECTIONS.
        float v = h<4 ? y : h==12||h==14 ? x : z;
        return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
    }
    __m256 Noise::_mm256_fade_ps(__m256 t)
    {
        __m256 c6 = _mm256_set1_ps(6.0f);
        __m256 c15 = _mm256_set1_ps(15.0f);
        __m256 c10 = _mm256_set1_ps(10.0f);
        return _mm256_mul_ps(t, _mm256_mul_ps(t, _mm256_mul_ps(t, _mm256_add_ps(_mm256_mul_ps(t, _mm256_sub_ps(_mm256_mul_ps(t, c6), c15)), c10))));
    }
    __m256 Noise::_mm256_lerp_ps(__m256 t, __m256 a, __m256 b)
    {
        return _mm256_add_ps(a, _mm256_mul_ps(t, _mm256_sub_ps(b, a)));
    }
    __m256 Noise::_mm256_grad_ps(__m256i hash, __m256 x, __m256 y, __m256 z)
    {
        // Constants for comparison
        __m256i c8 = _mm256_set1_epi32(8);
        __m256i c4 = _mm256_set1_epi32(4);
        __m256i c12 = _mm256_set1_epi32(12);
        __m256i c14 = _mm256_set1_epi32(14);

        // Mask the lower 4 bits of the hash

        __m256i h = _mm256_and_si256(hash, _mm256_set1_epi32(15));

        // Conditional selection for u: u = h < 8 ? x : y;
        __m256 mask_u = _mm256_castsi256_ps(_mm256_cmpgt_epi32(c8, h)); // c8 > h, equivalent to h < 8
        __m256 u = _mm256_blendv_ps(y, x, mask_u);

        // Conditional selection for v: v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        // v = (h<4) ? y : (h==12)||(h==14) ? x : z;
        __m256 mask_v1 = _mm256_castsi256_ps(_mm256_cmpgt_epi32(c4, h));
        __m256 mask_v2_12 = _mm256_castsi256_ps(_mm256_cmpeq_epi32(h, c12));
        __m256 mask_v2_14 = _mm256_castsi256_ps(_mm256_cmpeq_epi32(h, c14));
        __m256 mask_v2 = _mm256_or_ps(mask_v2_12, mask_v2_14);  // Combine masks for 12 and 14
        __m256 v = _mm256_blendv_ps(z, x, mask_v2);
        v = _mm256_blendv_ps(v, y, mask_v1);

        // Calculate the result: return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        // Create masks to check the bits in h
        __m256i c1 = _mm256_set1_epi32(1); // Mask for bit 0
        __m256i c2 = _mm256_set1_epi32(2); // Mask for bit 1

        // Check if (h & 1) == 0
        __m256i mask_u_2 = _mm256_and_si256(h, c1);
        __m256i cmp_mask_u_2 = _mm256_cmpeq_epi32(mask_u_2, _mm256_setzero_si256());

        // Check if (h & 2) == 0
        __m256i mask_v = _mm256_and_si256(h, c2);
        __m256i cmp_mask_v = _mm256_cmpeq_epi32(mask_v, _mm256_setzero_si256());

        // Convert integer masks to floating-point masks
        __m256 mask_u_2_ps = _mm256_castsi256_ps(cmp_mask_u_2);
        __m256 mask_v_ps = _mm256_castsi256_ps(cmp_mask_v);

        // Select u or -u based on the mask
        __m256 neg_u = _mm256_sub_ps(_mm256_setzero_ps(), u); // -u
        __m256 u_result = _mm256_blendv_ps(neg_u, u, mask_u_2_ps);

        // Select v or -v based on the mask
        __m256 neg_v = _mm256_sub_ps(_mm256_setzero_ps(), v); // -v
        __m256 v_result = _mm256_blendv_ps(neg_v, v, mask_v_ps);


        return _mm256_add_ps(u_result, v_result);
    }
}