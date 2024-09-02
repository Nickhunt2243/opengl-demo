//
// Created by admin on 6/8/2024.
//

#ifndef OPENGLDEMO_PLAYER_HPP
#define OPENGLDEMO_PLAYER_HPP


#include <iostream>
#include <unordered_set>
#include <cmath>
#include <mutex>

#include "entity.hpp"
#include "../misc/coordinate.hpp"
#include "../misc/globals.hpp"
#include "../../setup/camera.hpp"
#include "../../setup/window.hpp"
#include "../../setup/program.hpp"
#include "../../helpers/timer.hpp"

namespace Craft
{
    class Player: public Entity
    {
    public:
        Player(
            Engine::Timer* timer,
            Engine::Window* window,
            Engine::Program* blockProgram,
            Engine::Program* worldProgram,
            uint32_t width,
            uint32_t height,
            std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords,
            std::mutex* coordsMutex
        );
        /**
         * A function that initializes the players camera.
         *
         * @return True if the camera was initialized, otherwise False.
         */
        bool initPlayer();
        /// A method for updating the players state.
        Coordinate2D<int> updatePlayer();
        /// Retrieve the block located at lookAtBlock on the side that you are looking at.
        Coordinate<int> getNextLookAtBlock() const;
        /// The block the player is looking at.
        Coordinate<int>* lookAtBlock{};
        /// The side of the block the player is looking at (used later for placing blocks).
        BlockSideType lookAtSide{BlockSideType::NONE};
        /**
         * This is a really simple function that determines if the block being placed intersects with the players bounds.
         *
         * @return A Boolean of whether the block intersects with the player.
         */
        bool playerIntersectsBlock();
        inline Engine::Camera* getCamera() {
            return &camera;
        }
    private:
        /// The game timer.
        Engine::Timer* timer;
        /// The program for drawing blocks.
        Engine::Program* blockProgram;
        /// The walking speed of the player. Scientifically proven to be accurate.
        float cameraWalkingSpeedPerMilli{0.004317f};
        /// The vec3 describing the View matrices up direction.
        glm::vec3 cameraUp{glm::vec3(0.0f, 1.0f,  0.0f)};
        /// A mapping of chunk coords to a block placement bitmap for collision.
        std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords;
        /// A pointer to the GLFW window.
        Engine::Window* window;
        /// The camera of the scene.
        Engine::Camera camera;
        /// A mutex for accessing the coords mapping.
        std::mutex* coordsMutex;
        /**
         * Determine whether the t scalar value derived from the ray-AABB algorithm intersects within the bounds
         * of the current block.
         *
         * If it does, then we check if there is a block there.
         *  - If there is a block there then we have collided with a block and we are done.
         *  - If not, then we will continue the algorithm from this new block.
         * If the bounds do not intersect then we will return and check if another side of the block intersects
         * the ray (camera direction).
         *
         * @param t:        The current t (vector scalar) for the direction vector.
         * @param pos:      The current position of the players eyes in world coordinates.
         * @param normDir:  The normalized direction vector (players look direction normalized).
         * @param blockX:   The X coordinate of the current block to check.
         * @param blockY:   The Y coordinate of the current block to check.
         * @param blockZ:   The Z coordinate of the current block to check.
         * @param currSide: The intersection of the current block.
         * @param nextSide: The intersection of the previous block.
         * @return:         The distance from the closest block that intersects.
         */
        long double rayIntersection(long double t, glm::vec3 pos, glm::vec3 normDir, int blockX, int blockY, int blockZ, BlockSideType currSide, BlockSideType nextSide, int depth);
        /**
         * Simple algorithm I wrote using Ray-AABB collision algorithm from:
         *
         * https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
         *
         * Also view my 2D and 3D desmos demos to better visualize how we are determining vector collision within 3D
         * space:
         * 2D visualization (easier to take in and play around with):
         *      - https://www.desmos.com/calculator/wbt2b77klj
         * 3D visualization (more accurate to how we are utilizing this algo as it is 3D):
         *      - https://www.desmos.com/3d/hkujwxv5a2
         *
         * @param pos:      The current position of the players eyes in world coordinates.
         * @param normDir:  The normalized direction vector (players look direction normalized).
         * @param blockX:   The X coordinate of the current block to check.
         * @param blockY:   The Y coordinate of the current block to check.
         * @param blockZ:   The Z coordinate of the current block to check.
         * @param prevSide: The intersection of the previous block.
         * @return:         The distance from the closest block that intersects.
         */
        long double performRayAABB(glm::vec3 pos, glm::vec3 dir, int blockX, int blockY, int blockZ, BlockSideType prevSide, int depth);

        /// Simple function for calculating the Euclidean distance of two points in 3D space.
        inline static long double eucDistance(
                long double x1, long double x2,
                long double y1, long double y2,
                long double z1, long double z2
            )
        {
            long double xDiff = x1 - x2;
            long double yDiff = y1 - y2;
            long double zDiff = z1 - z2;

            return sqrt((xDiff) * (xDiff) + (yDiff) * (yDiff) + (zDiff) * (zDiff));
        }
    };
}

#endif //OPENGLDEMO_PLAYER_HPP
