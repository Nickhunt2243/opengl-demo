//
// Created by admin on 6/14/2024.
//

#ifndef OPENGLDEMO_NOISE_HPP
#define OPENGLDEMO_NOISE_HPP

#include <immintrin.h>

namespace Craft
{
    class Noise {
    public:
        Noise(uint32_t seed);
        float noise(float x, float y, float z) ;
        static float fade(float t);
        /// A function for calculating the linear interpolation between a and b.
        static float lerp(float t, float a, float b);
        static float grad(int hash, float x, float y, float z);
        // SIMD functions for noise
        static __m256 _mm256_fade_ps(__m256 t);
        static __m256 _mm256_lerp_ps(__m256 t, __m256 a, __m256 b);
        static __m256 _mm256_grad_ps(__m256i hash, __m256 x, __m256 y, __m256 z);
        __m256 _mm256_noise_ps(__m256 x, __m256 y, __m256 z);

        /**
         * Simple function for creating fractal noise from our noise function.
         *
         * @param x:           The X to generate noise for.
         * @param y:           The Y to generate noise for.
         * @param z:           The Z to generate noise for.
         * @param octaves:     The amount of time to compound the noise.
         * @param persistence: How much each each octave affects the final noise value.
         * @param amplitude:   The height of the noise.
         * @param frequency:   The "length" of the noise.
         * @return:            A "random" decimal value representing the noise.
         */
        float fractalNoise(
            float x,
            float y,
            float z,
            int octaves,
            float persistence,
            float amplitude,
            float frequency
        );
        /// SIMD version of the fractal noise algo above.
        __m256 __m256_fractalNoise_ps(
                __m256 x,
                __m256 y,
                __m256 z,
                int octaves,
                float persistence,
                float amplitude,
                float frequency
        );

    private:
        // Permutation vector
        std::vector<int> p;
    };

}

#endif //OPENGLDEMO_NOISE_HPP
