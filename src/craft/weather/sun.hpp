#ifndef OPENGLDEMO_SUN_HPP
#define OPENGLDEMO_SUN_HPP



#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>
#include <memory>
#include <iostream>

#include "../../helpers/timer.hpp"
#include "../misc/types.hpp"
#include "../misc/globals.hpp"
#include "../../setup/program.hpp"

namespace Craft
{
    class Sun
    {
    public:
        Sun(Engine::Program* worldProgram);
        ~Sun();
        /// Initialize the Suns VAO/VBO
        void initSun();
        /// Update the sun and moons model transformation.
        void updateSun();
        /// Draw the Sun and/or Moon.
        void drawLight(float playerX, float playerY, float playerZ);
        /// A getter for the in-game time.
        inline Time getTime()
        {
            return gameTime;
        }
    private:
        /// A pointer to a generic OpenGL program.
        Engine::Program* worldProgram;
        /// The timer for the sun/moon/game (in-game time).
        Engine::Timer clockTimer{};
        /// The time within the game.
        Time gameTime {8, 0, 0.0f};
        /// The angle of the sun.
        float sunAngle {0};
        /// The angle of the moon.
        float moonAngle {0};
        /// Whether to draw the sun or not.
        bool drawSun{false};
        /// Whether to draw the moon or not.
        bool drawMoon{false};
        /// The color of the Sky.
        glm::vec3 skyColor{1.0f, 1.0f, 1.0f};
        /// The VAO and VBO
        GLuint VAO{0}, VBO{0};
        /// The vertices of the solar objects.
        std::vector<float> solarObjectVertices
        {
            0, 0.5f, -0.5f,
            0, -0.5f, -0.5f,
            0, 0.5f, 0.5f,
            0, 0.5f, 0.5f,
            0, -0.5f, -0.5f,
            0, -0.5f, 0.5f
        };
        /// The color of the sun (will eventually use textures).
        glm::vec4 sunColor {1.0f, 1.0f, 0.85f, 1.0f};
        /// The color of the moon (will eventually use textures).
        glm::vec4 moonColor {0.85f, 0.85f, 0.85f, 1.0f};
        /**
         *
         * @param playerX: The players X value.
         * @param playerY:
         * @param playerZ: The players Z value.
         */
        void updateModel(float objectX, float objectY, float objectZ, float angle);
        /**
         * Calculate the color of the sky based on actual minecraft sky colors:
         *
         * For more information and a visual of how this is done, view:
         * https://www.desmos.com/calculator/d90zr0hskc
         */
        void calcSkyColor();
        /// Calculate the new time that is based on the offset of the last time this function was called.
        void calcNewTime();
        /**
         * Calculate the angle of the sun and moon and determines whether or not to
         * draw them.
         *
         * For more information and a visual of how this is done, view:
         * https://www.desmos.com/calculator/ek8vw7jwfh
         *
         */
        void calcNewAngle();
    };
}

#endif //OPENGLDEMO_SUN_HPP
