//
// Created by admin on 6/8/2024.
//

#include "player.hpp"
#include "../../helpers/helpers.hpp"

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_4
#define M_PI_4 (M_PI / 4.0f)
#endif
#ifndef MAX_BLOCKS
#define MAX_BLOCKS (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT)
#endif
#define JUMP_HEIGHT 1.25f
#define PLAYER_EYE_DIFF 0.38f
#define REACH_DISTANCE 4.5l

namespace Craft {
    Player::Player(
            Engine::Timer *timer,
            Engine::Window *window,
            Engine::Program *blockProgram,
            Engine::Program *worldProgram,
            unsigned int width,
            unsigned int height,
            std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> *> *coords,
            std::mutex* coordsMutex
    )
            : timer{timer}
            , blockProgram{blockProgram}
            , coords{coords}
            , window{window}
            , coordsMutex{coordsMutex}
            , Entity
            {
                    timer,
                    1.5l, 101.0l, 13.5l,
                    Coordinate2D<int>{0, 0},
                    PLAYER_FRONT_BOUND, PLAYER_BACK_BOUND, PLAYER_LEFT_BOUND, PLAYER_RIGHT_BOUND,
                    coords
            }
            , camera
            {
                  window, blockProgram, worldProgram, width, height,
                  (float) entityX, (float) entityY, (float) entityZ
            } {}

    bool Player::initPlayer() {
        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera()) {
            return false;
        }

        timer->startStopWatch();

        return true;
    }

    Coordinate2D<int> Player::updatePlayer() {
        glm::vec3 cameraPos = glm::vec3{entityX + (originChunk.x * 16), entityY, entityZ + (originChunk.z * 16)};
        glm::vec3 cameraFront = camera.getCameraFront();

        float angle = atan2(cameraFront.x, cameraFront.z);
        Coordinate2D<int> playerDirection{0, 0};
        if (angle > -M_PI_4 && angle <= M_PI_4)
        {
            playerDirection.z = 1;
        } else if (angle > M_PI_4 && angle <= 3 * M_PI_4)
        {
            playerDirection.x = 1;
        } else if ((angle > 3 * M_PI_4 && angle <= M_PI) || (angle >= -M_PI && angle <= -3 * M_PI_4))
        {
            playerDirection.z = -1;
        } else if (angle > -3 * M_PI_4 && angle <= -M_PI_4)
        {
            playerDirection.x = -1;
        }

        float milliSinceLastUpdate = timer->getTimeSpan();
        float walkingSpeed = cameraWalkingSpeedPerMilli * milliSinceLastUpdate;
        glm::vec3 movementVec{0.0f};
        // Move Forward
        if (glfwGetKey(window->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            movementVec += walkingSpeed * glm::normalize(glm::vec3{cameraFront.x, 0.0f, cameraFront.z});
        }
        // Move back
        if (glfwGetKey(window->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            movementVec -= walkingSpeed * glm::normalize(glm::vec3{cameraFront.x, 0.0f, cameraFront.z});
        }
        // Move left
        if (glfwGetKey(window->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            movementVec -= walkingSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
        }
        // Move right
        if (glfwGetKey(window->getWindow(), GLFW_KEY_D) == GLFW_PRESS) {
            movementVec += walkingSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
        }
        // toggle flying
        if (glfwGetKey(window->getWindow(), GLFW_KEY_F) == GLFW_PRESS)
        {
            if (vertMovement == EntityVertMovementType::FLYING)
            {
                startFalling();
            }
            else
            {
                vertMovement = EntityVertMovementType::FLYING;
            }
        }

        entityX += movementVec.x;
        entityZ += movementVec.z;
        if (movementVec.x != 0 || movementVec.z != 0) {
            Coordinate2D<long double> correction = entityCollidedHeadOn(playerDirection.x, playerDirection.z);

            if (correction.x == 0 && correction.z == 0) {
                correction = entityCollidedAtCorner(playerDirection.x, playerDirection.z);
                bool movingX = playerDirection.x != 0;
                float xMag = abs(movementVec.x);
                float zMag = abs(movementVec.z);
                bool walkingMoreXDir = xMag > zMag;
                // If we are going forward then
                if (!movingX && walkingMoreXDir || movingX && !walkingMoreXDir) {
                    long double tmp = correction.x;
                    correction.x = correction.z;
                    correction.z = tmp;
                }
            }
            if (correction.x != 0) {
                entityX += correction.x;
                movementVec.x += (float) correction.x;
            }
            if (correction.z != 0) {
                entityZ += correction.z;
                movementVec.z += (float) correction.z;
            }
            cameraPos += movementVec;
        }

        // Jump
        if (glfwGetKey(window->getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            // Start jumping
            if (vertMovement == EntityVertMovementType::FLYING)
            {
                cameraPos.y += 0.01f * milliSinceLastUpdate;
                entityY = cameraPos.y;
            }
            else if (vertMovement == EntityVertMovementType::STATIONARY)
            {
                vertMovement = EntityVertMovementType::JUMPING;
                initialJumpHeight = floor(entityY);
                timer->startStopWatch();
            }
        }
        // Move down (will eventually remove, maybe implement crouch.)
        if (
                vertMovement == EntityVertMovementType::FLYING &&
                glfwGetKey(window->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
            )
        {
            if (blockBelowEntity(angle))
            {
                vertMovement = EntityVertMovementType::STATIONARY;
            }
            else
            {
                cameraPos.y += -0.01f * milliSinceLastUpdate;
                entityY = cameraPos.y;
            }
        }

        if (vertMovement == EntityVertMovementType::JUMPING && entityY < initialJumpHeight + JUMP_HEIGHT)
        {
            float newY = initialJumpHeight + calcJumpDisplacement();
            cameraPos.y = newY;
            entityY = cameraPos.y;
        }
        else if (vertMovement == EntityVertMovementType::JUMPING && entityY >= initialJumpHeight + JUMP_HEIGHT)
        {
            // after jumping go into falling.
            startFalling();
        }

        if (vertMovement == EntityVertMovementType::FALLING)
        {
            bool stopFall = blockBelowEntity(angle);
            if (stopFall)
            {
                entityY = round(entityY);
                vertMovement = EntityVertMovementType::STATIONARY;
            }
            else
            {
                float newY = (float) initialFallHeight + calcFallDisplacement();
                cameraPos.y = newY;
                entityY = cameraPos.y;
            }
        }
        else if (
                vertMovement != EntityVertMovementType::FLYING &&
                vertMovement == EntityVertMovementType::STATIONARY
            )
        {
            bool stopFall = blockBelowEntity(angle);
            if (!stopFall)
            {
                startFalling();
            }
        }

        cameraPos.y -= PLAYER_EYE_DIFF;
        performRayAABB(cameraPos, glm::normalize(cameraFront), floor(cameraPos.x), floor(cameraPos.y), floor(cameraPos.z), BlockSideType::NONE);
//        std::cout << "Pos: " << Coordinate<float>{cameraPos.x, cameraPos.y, cameraPos.z} << std::endl;
//        std::cout << "Pos: " << Coordinate<int>{(int)floor(cameraPos.x), (int)floor(cameraPos.y), (int)floor(cameraPos.z)} << std::endl;
//        std::cout << Coordinate<float>{glm::normalize(cameraFront).x, glm::normalize(cameraFront).y, glm::normalize(cameraFront).z} << std::endl;
        blockProgram->useProgram();
        if (lookAtBlock != nullptr)
        {
//            std::cout << "Look at: " << *lookAtBlock << std::endl;
            setVec3(blockProgram->getProgram(), "u_lookAtBlock", *lookAtBlock);
            setBool(blockProgram->getProgram(), "u_hasLookAt", true);
        }
        else
        {
            setBool(blockProgram->getProgram(), "u_hasLookAt", false);

        }
        // Update the camera.
        if (!camera.updateCamera(cameraPos, cameraUp)) {
            // Since the diff will only be 0 or 1 we can use -2 as a way to catch failure.
            return {-2, -2};
        }

        entityX = fmod(cameraPos.x, 16.0f);
        if (entityX < 0)
        {
            entityX += 16.0f;
        }
        entityZ = fmod(cameraPos.z, 16.0f);
        if (entityZ < 0)
        {
            entityZ += 16.0f;
        }
        Coordinate2D<int> diff{0, 0};
        // Player moving in X direction
        if (cameraPos.x > (float) (originChunk.x * 16) + 16.0f)
        {
            diff.x = 1;
        }
        // Player moving in -X direction
        else if (cameraPos.x < (float) (originChunk.x * 16))
        {
            diff.x = -1;
        }
        // Player moving in Z direction
        else if (cameraPos.z > (float) (originChunk.z * 16) + 16.0f)
        {
            diff.z = 1;
        }
        // Player moving in -Z direction
        else if (cameraPos.z < (float) (originChunk.z * 16))
        {
            diff.z = -1;
        }
        originChunk = originChunk + diff;

        return diff;
    }
    Coordinate<int> getNextBlock(BlockSideType intersectedSide)
    {
        if (intersectedSide == BlockSideType::X_MAX) return {1, 0, 0};
        else if (intersectedSide == BlockSideType::X_MIN) return {-1, 0, 0};
        else if (intersectedSide == BlockSideType::Y_MAX) return {0, 1, 0};
        else if (intersectedSide == BlockSideType::Y_MIN) return {0, -1, 0};
        else if (intersectedSide == BlockSideType::Z_MAX) return {0, 0, 1};
        else if (intersectedSide == BlockSideType::Z_MIN) return {0, 0, -1};
    }
    long double Player::rayIntersection(
            long double t,
            glm::vec3 pos,
            glm::vec3 normDir,
            int blockX, int blockY, int blockZ,
            BlockSideType currSide, BlockSideType nextSide
        )
    {
        // Intersection will be behind player
        if (t < 0 || t == std::numeric_limits<long double>::infinity() || t == -std::numeric_limits<long double>::infinity())
        {
            return 0.0l;
        }
        // Find intersection points in both chunk and world coordinates.
        long double intersectX = t * normDir.x + pos.x;
        long double intersectY = t * normDir.y + pos.y;
        long double intersectZ = t * normDir.z + pos.z;
        long double distFromPlayer = eucDistance(intersectX, getWorldX(), intersectY, pos.y, intersectZ, getWorldZ());
        if (
                intersectX + FLT_EPSILON >= blockX && intersectX - FLT_EPSILON <= blockX + 1 &&
                intersectY + FLT_EPSILON >= blockY && intersectY - FLT_EPSILON <= blockY + 1 &&
                intersectZ + FLT_EPSILON >= blockZ && intersectZ - FLT_EPSILON <= blockZ + 1 &&
                distFromPlayer <= REACH_DISTANCE
            )
        {
            Coordinate<int> nextBlockDiff = getNextBlock(nextSide);
            int nextBlockX = blockX + nextBlockDiff.x;
            int nextBlockY = blockY + nextBlockDiff.y;
            int nextBlockZ = blockZ + nextBlockDiff.z;
            Coordinate2D<int> chunkPos{(int) floor(nextBlockX / 16), (int) floor(nextBlockZ / 16)};
            int chunkBlockX = nextBlockX - (chunkPos.x * 16);
            int chunkBlockZ = nextBlockZ - (chunkPos.z * 16);
            blockInfo info = getBlockInfo(chunkBlockX, chunkBlockZ, chunkPos);
            // Checking if is block.
            int blockIdx = (nextBlockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;
            bool isBlock;
            {
                std::lock_guard<std::mutex> lock(*coordsMutex);
                isBlock = (blockIdx >= 0 && blockIdx < MAX_BLOCKS && (*coords->at(info.chunk))[blockIdx]);
            }
            if (
                    isBlock && (lookAtBlock == nullptr ||
                    lookAtBlock->x != nextBlockX || lookAtBlock->y != nextBlockY || lookAtBlock->z != nextBlockZ)
                )
            {
                // Player lookAtBlock
                if (lookAtBlock == nullptr)
                {
                    lookAtBlock = new Coordinate(nextBlockX, nextBlockY, nextBlockZ);
                }
                else
                {
                    lookAtBlock->x = nextBlockX;
                    lookAtBlock->y = nextBlockY;
                    lookAtBlock->z = nextBlockZ;
                }
                lookAtSide = currSide;
                return distFromPlayer;
            }
            else if (isBlock && lookAtBlock->x == nextBlockX && lookAtBlock->y== nextBlockY && lookAtBlock->z == nextBlockZ)
            {
                return distFromPlayer;
            }
            else
            {
                return performRayAABB(pos, normDir, nextBlockX, nextBlockY, nextBlockZ, nextSide);
            }
        }
        // return 0.0l meaning there was no intersection within bounds, so we will not count this as an intersection.
        return 0.0l;
    }

    long double Player::performRayAABB(glm::vec3 pos, glm::vec3 normDir, int blockX, int blockY, int blockZ, BlockSideType prevSide)
    {
        // Remember to
        long double t;
        long double rayIntersectionDist = 0.0f;
        // If we enter from x max then we will not check x_min since it is the same side, etc.
        if (prevSide != BlockSideType::X_MAX)
        {
            // Checking x_min
            t = (blockX - pos.x) / normDir.x;
            rayIntersectionDist = rayIntersection(
                    t, pos, normDir, blockX, blockY, blockZ, BlockSideType::X_MAX, BlockSideType::X_MIN
            );
        }
        if (prevSide != BlockSideType::X_MIN && rayIntersectionDist == 0)
        {
            // Checking x_max
            t = ((blockX + 1) - pos.x) / normDir.x;
            rayIntersectionDist = rayIntersection(
                    t, pos, normDir, blockX, blockY, blockZ, BlockSideType::X_MIN, BlockSideType::X_MAX
            );
        }
        if (prevSide != BlockSideType::Y_MAX && rayIntersectionDist == 0.0l)
        {
            // Checking y_min
            t = (blockY - pos.y) / normDir.y;
            rayIntersectionDist = rayIntersection(
                    t, pos, normDir, blockX, blockY, blockZ, BlockSideType::Y_MAX, BlockSideType::Y_MIN
            );
        }
        if (prevSide != BlockSideType::Y_MIN && rayIntersectionDist == 0.0l)
        {
            // Checking z_max
            t = ((blockY + 1) - pos.y) / normDir.y;
            rayIntersectionDist = rayIntersection(
                    t, pos, normDir, blockX, blockY, blockZ, BlockSideType::Y_MIN, BlockSideType::Y_MAX
            );
        }
        if (prevSide != BlockSideType::Z_MAX && rayIntersectionDist == 0.0l)
        {
            // Checking z_min
            t = (blockZ - pos.z) / normDir.z;
            rayIntersectionDist = rayIntersection(
                    t, pos, normDir, blockX, blockY, blockZ, BlockSideType::Z_MAX, BlockSideType::Z_MIN
            );
        }
        if (prevSide != BlockSideType::Z_MIN && rayIntersectionDist == 0.0l)
        {
            // Checking z_max
            t = ((blockZ + 1) - pos.z) / normDir.z;
            rayIntersectionDist = rayIntersection(
                    t, pos, normDir, blockX, blockY, blockZ, BlockSideType::Z_MIN, BlockSideType::Z_MAX
            );
        }

        if ((rayIntersectionDist > REACH_DISTANCE || rayIntersectionDist == 0.0l) && lookAtBlock != nullptr)
        {
            delete lookAtBlock;
            lookAtBlock = nullptr;
            lookAtSide = BlockSideType::NONE;
        }

        return rayIntersectionDist;


    }

//    Coordinate<float> detectBlockCollision()
//    {
//
//    }
}