//
// Created by admin on 6/14/2024.
//

#ifndef OPENGLDEMO_NOISE_HPP
#define OPENGLDEMO_NOISE_HPP

namespace Craft
{
    class Noise {
    public:
        Noise(unsigned int seed);
        float noise(float x, float y, float z) ;
        float fade(float t);
        /// A function for calculating the linear interpolation between a and b.
        float lerp(float t, float a, float b);
        float grad(int hash, float x, float y, float z);
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
    private:
        // Permutation vector
        std::vector<int> p;
    };

}

#endif //OPENGLDEMO_NOISE_HPP
