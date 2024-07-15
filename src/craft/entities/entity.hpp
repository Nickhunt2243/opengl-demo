//
// Created by admin on 7/3/2024.
//

#ifndef OPENGLDEMO_ENTITY_HPP
#define OPENGLDEMO_ENTITY_HPP

#ifndef CHUNK_OFFSET
#define CHUNK_OFFSET 0.5f
#endif
#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif
#ifndef CHUNK_HEIGHT
#define CHUNK_HEIGHT 128
#endif

#include <bitset>
#include <mutex>

#include "../misc/coordinate.hpp"
#include "../../helpers/timer.hpp"
#include "../../setup/window.hpp"
#include "../../setup/program.hpp"
#include "../misc/types.hpp"

namespace Craft
{
    class Entity
    {
    public:
        Entity(
            Engine::Timer* timer,
            long double x, long double y, long double z,
            Coordinate2D<int> chunkPos,
            long double front, long double back, long double left, long double right,
            std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords
        );
        ~Entity() = default;
        /// The entity's X, Y, and Z coordinates.
        long double entityX,
                    entityY,
                    entityZ;
        /// The origin chunk of this entity.
        Coordinate2D<int> originChunk;
        /// An enum denoting the various vertical movements.
        enum class EntityVertMovementType {
            JUMPING,
            FALLING,
            STATIONARY,
            FLYING
        };
        /// A struct holding the entity bounds.
        struct EntityBounds{
            long double front;
            long double back;
            long double left;
            long double right;
        };
        [[nodiscard]] inline long double getWorldX() const
        {
            return (long double) (originChunk.x * 16) + entityX;
        }
        [[nodiscard]] inline long double getWorldZ() const
        {
            return (long double) (originChunk.z * 16) + entityZ;
        }
    protected:
        /// The game timer.
        Engine::Timer* timer;
        /// The type of vertical movement the player is experiencing.
        EntityVertMovementType vertMovement{EntityVertMovementType::STATIONARY};
        /// The bounds of the given entity.
        EntityBounds entityBounds;
        /**
         * The value of the players initial jump value. Integer since you can only jump from a block.
         *
         * NOTE: Will eventually change as stairs and other blocks will have decimal solid surfaces.
         */
        float initialJumpHeight{-1};
        /// The initial fall height of the player. Used for falling with a displacement.
        long double initialFallHeight{-1};
        /// A mapping of chunk coords to a block placement bitmap for collision.
        std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords;
        /**
         * A function for telling if the players coordinates are on top of a block.
         *
         * We check two things. if there are any blocks at the current Y that have vertices within the bounds of the
         * player and if any of the player's vertices are within the bounds of any of the blocks.
         *
         * Desmos visual example found: https://www.desmos.com/calculator/upydgsvggu
         *
         * @param angle: The angle the player is rotated.
         * @return True if the player is on a block.
         */
        bool blockBelowEntity(float angle);
        /**
         * Retrieve a given block's X, Z, and chunk coordinate normalized in 0-15 indexing.
         *
         * @param blockX:   The current block X value.
         * @param blockZ:   The current block Z value.
         * @param chunkPos: The current chunk position.
         * @return:       A struct containing the blocks x, z, and chunk coordinate.
         */
        blockInfo getBlockInfo(int blockX, int blockZ, Coordinate2D<int> chunkPos);
        /**
         * A method to handle x/z head on collision.
         *
         * For more information and a visual of how we handle this view:
         * https://www.desmos.com/calculator/k6fzl1xo3n
         *
         * @param xDirection: 1 if we are moving in +X direction, -1 if -X direction and 0 if moving in Z direction.
         * @param zDirection: 1 if we are moving in +Z direction, -1 if -Z direction and 0 if moving in X direction.
         * @return:           A Coordinate2D holding the x and z correction if colliding with another block.
         */
        Coordinate2D<long double> entityCollidedHeadOn(int xDirection, int zDirection);
        /**
         * A method to handle x/z corner collision.
         *
         * For more information and a visual of how we handle this view:
         * https://www.desmos.com/calculator/ffvcpr5cx6
         *
         * @param xDirection: 1 if we are moving in +X direction, -1 if -X direction and 0 if moving in Z direction.
         * @param zDirection: 1 if we are moving in +Z direction, -1 if -Z direction and 0 if moving in X direction.
         * @return:           A Coordinate2D holding the x and z correction if colliding with another block.
         */
        Coordinate2D<long double> entityCollidedAtCorner(int xDirection, int zDirection);

        /**
         * Calculate the Y displacement overtime when falling.
         *
         * Equation found at:  https://minecraft.fandom.com/wiki/Transportation#:~:text=Falling%20speed%20is%20more%20complex,from%20%22above%20the%20sky%22.
         *
         * @return The current displacement.
         */
        inline float calcFallDisplacement()
        {
            float elapsedFallTime = timer->lapStopWatch();
            float x = (elapsedFallTime / 1000);
            return 392.0f * (0.5f - 0.2f * x - ( 0.5f * pow(49.0f / 50.0f, 20.0f * x)));
        }
        /**
         * An equation that calculates the player's displacement when jumping.
         *
         * Allows player to jump 1.25 height in .325 seconds smoothly. Something I just randomly chose.
         *
         * @return: The current displacement.
         */
        inline float calcJumpDisplacement()
        {
            float elapsedFallTime = timer->lapStopWatch();
            float x = (elapsedFallTime / 1000);
            return -pow(1.8f, -13.0f * (x - 0.045f)) + 1.4f;
        }
        /**
         * Calculate the X value given an angle of rotation.
         *
         * @param x:     The X value prior to rotation.
         * @param z:     The Z value prior to rotation.
         * @param angle: The angle of rotation.
         * @return:      The rotated X value.
         */
        static inline long double calcRotatedX(long double x, long double z, float angle)
        {
            return x * cos(angle) + z * sin(angle);
        }
        /**
         * Calculate the Z value given an angle of rotation.
         *
         * @param x:     The X value prior to rotation.
         * @param z:     The Z value prior to rotation.
         * @param angle: The angle of rotation.
         * @return:      The rotated Z value.
         */
        static inline long double calcRotatedZ(long double x, long double z, float angle)
        {
            return -x * sin(angle) + z * cos(angle);
        }
        /**
         * Find the euclidean distance from two points.
         *
         * @param x1: First points X value.
         * @param z1: First points Z value.
         * @param x2: Second points X value.
         * @param z2: Second points Z value.
         * @return:   The Euclidean distance.
         */
        static inline long double eucDistance(long double x1, long double z1, long double x2, long double z2)
        {
            long double x = x1 - x2;
            long double z = z1 - z2;
            return sqrt((x * x) + (z * z));
        }
        /// Helper function to start the falling workflow.
        inline void startFalling()
        {
            timer->startStopWatch();
            vertMovement = EntityVertMovementType::FALLING;
            initialFallHeight = entityY;
        }
    };
}

#endif //OPENGLDEMO_ENTITY_HPP
