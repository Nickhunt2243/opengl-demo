//
// Created by admin on 6/8/2024.
//

#ifndef OPENGLDEMO_PLAYER_HPP
#define OPENGLDEMO_PLAYER_HPP

#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif
#ifndef CHUNK_HEIGHT
#define CHUNK_HEIGHT 128
#endif

#include <iostream>
#include <unordered_set>
#include <cmath>
#include <mutex>
#include <bitset>

#include "../setup/camera.hpp"
#include "../setup/window.hpp"
#include "../setup/program.hpp"
#include "coordinate.hpp"

namespace Craft
{
    class Player
    {
    public:
        Player(
            Engine::Window* window,
            Engine::Program* program,
            unsigned int width,
            unsigned int height,
            std::unordered_map<Coordinate2D, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords,
            std::mutex* coordsMutex
        );
        /**
         * A function that initializes the players camera.
         *
         * @return True if the camera was initialized, otherwise False.
         */
        bool initPlayer();
        /**
         * Updates the players position and what they are looking at.
         *
         * Added a simple form of physics "if player not above a block, then player is falling."
         *
         * @return True if the player was updates, otherwise False.
         */
        bool updatePlayer();
        /// The players X, Y, and Z coordinates.
        float playerX,
              playerY,
              playerZ;
    private:
        /// The walking speed of the camera.
        float cameraWalkingSpeed{0.2f};
        /// The height of the player.
        float playerHeight{2.0f};
        /// The width of player from side to side.
        float playerWidthLR{0.35f};
        /// The width of the player from front to back.
        float playerWidthFB{0.6f};
        /// The falling speed of the player.
        float fallingSpeed{0.1f};
        /// The vec3 describing the View matrices up direction.
        glm::vec3 cameraUp{glm::vec3(0.0f, 1.0f,  0.0f)};
        /// A mapping of chunk coords to a block placement bitmap for collision.
        std::unordered_map<Coordinate2D, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords;
        /// Boolean for whether the player is falling or not.
        bool isFalling{true};
        /// A pointer to the GLFW window.
        Engine::Window* window;
        /// A pointer to a set of 3D coordinates.
//        std::unordered_set<size_t>* coords;
        /// The camera of the scene.
        Engine::Camera camera;
        /// Mutex for the coords set.
        std::mutex* coordsMutex;
        /**
         * A function for telling if the players coordinates are on top of a block.
         *
         * We round the players coordinates and check if there is a block for that coordinate.
         *
         * NOTE: This will need to be updated when we add differently shaped blocks such as stairs.
         *
         * @return True if the player is on a block.
         */
        bool blockBelowPlayer();

        /// TODO: Implement left, right, back, and forward collision. Will implement after Vertex Buffer refactor.
        //  bool blockInFrontPlayer();
        //  bool blockBehindPlayer();
        //  bool blockLeftOfPlayer();
        //  bool blockRightOfPlayer();
    };
}

#endif //OPENGLDEMO_PLAYER_HPP
