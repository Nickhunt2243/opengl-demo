//
// Created by admin on 6/8/2024.
//

#include "player.hpp"

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_4
#define M_PI_4 (M_PI / 4.0f)
#endif
#define JUMP_HEIGHT 1.25f

namespace Craft {
    Player::Player(
            Engine::Timer* timer,
            Engine::Window* window,
            Engine::Program* program,
            unsigned int width,
            unsigned int height,
            std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords
    )
        : timer{timer}
        , coords{coords}
        , window{window}
        , Entity
                {
                    timer,
                    9.0l, 102.0l, 8l,
                    Coordinate2D<int>{0, 0},
                    PLAYER_FRONT_BOUND, PLAYER_BACK_BOUND, PLAYER_LEFT_BOUND, PLAYER_RIGHT_BOUND,
                    coords
                }
        , camera
                {
                      window, program, width, height,
                      (float) entityX, (float) entityY, (float) entityZ
                }
    {}

    bool Player::initPlayer()
    {
        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera())
        {
            return false;
        }

        timer->startStopWatch();

        return true;
    }

    Coordinate2D<int> Player::updatePlayer()
    {
        glm::vec3 cameraPos = glm::vec3{entityX + (originChunk.x * 16), entityY, entityZ + (originChunk.z * 16)};
        glm::vec3 cameraFront = camera.getCameraFront();

        float angle = atan2(cameraFront.x, cameraFront.z);
        Coordinate2D<int> playerDirection{0, 0};
        if (angle > -M_PI_4 && angle <= M_PI_4)
        {
            playerDirection.z = 1;
        }
        else if (angle > M_PI_4 && angle <= 3 * M_PI_4)
        {
            playerDirection.x = 1;
        }
        else if ((angle > 3 * M_PI_4 && angle <= M_PI) || (angle >= -M_PI && angle <= -3 * M_PI_4))
        {
            playerDirection.z = -1;
        }
        else if (angle > -3 * M_PI_4 && angle <= -M_PI_4)
        {
            playerDirection.x = -1;
        }

        float secondsSinceLastUpdate = timer->getTimeSpan();
        float walkingSpeed = cameraWalkingSpeedPerMilli * secondsSinceLastUpdate * 2;
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
        if (glfwGetKey(window->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            movementVec += walkingSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
        }
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
            // toggle flying
        }

        entityX += movementVec.x;
        entityZ += movementVec.z;
        if (movementVec.x != 0 || movementVec.z != 0) {
            Coordinate2D<long double> correction = entityCollidedWithBlock(playerDirection.x, playerDirection.z);

            if (correction.x != 0)
            {
                entityX += correction.x;
                movementVec.x += (float) correction.x;
            }
            if (correction.z != 0)
            {
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
                cameraPos.y += 0.01f * secondsSinceLastUpdate;
                entityY = cameraPos.y;
            }
            else if (vertMovement == EntityVertMovementType::STATIONARY)
            {
                vertMovement = EntityVertMovementType::JUMPING;
                initialJumpHeight = (int) floor(entityY);
                timer->startStopWatch();
            }
        }
        // Move down (will eventually remove, maybe implement crouch.)
        if (vertMovement == EntityVertMovementType::FLYING && glfwGetKey(window->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            if (blockBelowEntity(angle)) {
                vertMovement = EntityVertMovementType::STATIONARY;
            }
            else
            {
                cameraPos.y += -0.01f * secondsSinceLastUpdate;
                entityY = cameraPos.y;
            }
        }

        if (vertMovement == EntityVertMovementType::JUMPING && entityY < initialJumpHeight + JUMP_HEIGHT)
        {
//            std::cout << "Init: " << initialJumpHeight << ", current: " << cameraPos.y << std::endl;
            float newY = initialJumpHeight + calcJumpDisplacement();
            cameraPos.y = newY;
            entityY = cameraPos.y;
        }
        else if (vertMovement == EntityVertMovementType::JUMPING && entityY >= initialJumpHeight + JUMP_HEIGHT)
        {
            // after jumping go into falling.
           startFalling();
        }

        if (vertMovement == EntityVertMovementType::FALLING) {
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
        else if (vertMovement != EntityVertMovementType::FLYING && vertMovement == EntityVertMovementType::STATIONARY)
        {
            bool stopFall = blockBelowEntity(angle);
            if (!stopFall)
            {
                startFalling();
            }
        }

        // Update the camera.
        if (!camera.updateCamera(cameraPos, cameraUp))
        {
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
        else if (cameraPos.x < (float)  (originChunk.x * 16))
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
}