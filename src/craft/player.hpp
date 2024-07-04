//
// Created by admin on 6/8/2024.
//

#ifndef OPENGLDEMO_PLAYER_HPP
#define OPENGLDEMO_PLAYER_HPP
#ifndef CHUNK_OFFSET
#define CHUNK_OFFSET 0.5f
#endif
#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif
#ifndef CHUNK_HEIGHT
#define CHUNK_HEIGHT 128
#endif

#define PLAYER_FRONT_BOUND 0.15l
#define PLAYER_BACK_BOUND 0.15l
#define PLAYER_LEFT_BOUND 0.3l
#define PLAYER_RIGHT_BOUND 0.3l


#include <iostream>
#include <unordered_set>
#include <cmath>
#include <mutex>
#include <bitset>

#include "entity.hpp"
#include "coordinate.hpp"
#include "../setup/camera.hpp"
#include "../setup/window.hpp"
#include "../setup/program.hpp"
#include "../helpers/timer.hpp"

namespace Craft
{
    class Player: public Entity
    {
    public:
        Player(
            Engine::Timer* timer,
            Engine::Window* window,
            Engine::Program* program,
            unsigned int width,
            unsigned int height,
            std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords
        );
        /**
         * A function that initializes the players camera.
         *
         * @return True if the camera was initialized, otherwise False.
         */
        bool initPlayer();
        /// A method for updating the players state.
        Coordinate2D<int> updatePlayer();
    private:
        /// The game timer.
        Engine::Timer* timer;
        /// The walking speed of the player. Not scientifically proven to be accurate.
        float cameraWalkingSpeedPerMilli{0.00356f};
        /// The vec3 describing the View matrices up direction.
        glm::vec3 cameraUp{glm::vec3(0.0f, 1.0f,  0.0f)};
        /// A mapping of chunk coords to a block placement bitmap for collision.
        std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords;
        /// A pointer to the GLFW window.
        Engine::Window* window;
        /// The camera of the scene.
        Engine::Camera camera;
    };
}

#endif //OPENGLDEMO_PLAYER_HPP
