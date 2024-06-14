//
// Created by admin on 6/8/2024.
//

#include "player.hpp"

#include <utility>

namespace Craft {
    Player::Player(
            Engine::Window* window,
            Engine::Program* program,
            unsigned int width,
            unsigned int height,
            std::unordered_map<Coordinate2D, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords,
            std::mutex* coordsMutex
    )
        : coords{coords}
        , playerX{0.0f}
        , playerY{130.0f}
        , playerZ{0.0f}
        , window{window}
        , camera{window, program, width, height, playerX, playerY, playerZ}
        , coordsMutex{coordsMutex}
    {}

    bool Player::initPlayer()
    {
        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera())
        {
            return false;
        }

        return true;
    }
    bool Player::blockBelowPlayer()
    {
//        float roundedX = round(playerX),
//              roundedY = round(playerY),
//              roundedZ = round(playerZ);
//
//        Coordinate playerCoord{roundedX, roundedY, roundedZ};
//        size_t altCoord = playerCoord.add(0.0f, -playerHeight, 0.0f);
//        std::lock_guard<std::mutex> lock(*coordsMutex);
//        return coords->find(altCoord) != coords->end();
    return true;
    }
//    bool Player::blockInFrontPlayer()
//    {
//        float roundedX = round(playerX),
//              roundedY = round(playerY),
//              roundedZ = round(playerZ);
//
//        Coordinate playerCoord{roundedX, roundedY, roundedZ};
//        size_t altCoord = playerCoord.add(playerWidthFB, -playerHeight/2, 0.0f);
//        std::lock_guard<std::mutex> lock(*coordsMutex);
//        return coords->find(altCoord) != coords->end();
//    }
    bool Player::updatePlayer()
    {
        glm::vec3 cameraPos = glm::vec3{playerX, playerY, playerZ};
        glm::vec3 cameraFront = camera.getCameraFront();
        // Move Forward
        if (glfwGetKey(window->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos += cameraWalkingSpeed * glm::vec3{cameraFront.x, 0.0, cameraFront.z};
        }
        // Move back
        if (glfwGetKey(window->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos -= cameraWalkingSpeed * glm::vec3{cameraFront.x, 0.0, cameraFront.z};
        }
        // Move left
        if (glfwGetKey(window->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraWalkingSpeed;
        }
        // Move right
        if (glfwGetKey(window->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraWalkingSpeed;
        }
        // Jump
        if (glfwGetKey(window->getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraWalkingSpeed;
        }
        // Move down (will eventually remove, maybe implement crouch.)
        if (glfwGetKey(window->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            cameraPos += glm::vec3(0.0f, -1.0f, 0.0f) * cameraWalkingSpeed;
        }

//        bool stopFall = blockBelowPlayer();
//
//        if (isFalling && stopFall)
//        {
//            isFalling = false;
//        }
//        else if (isFalling && !stopFall)
//        {
//            cameraPos.y -= fallingSpeed;
//        }
//        else if (!isFalling && !stopFall)
//        {
//            isFalling = true;
//        }

        playerX = cameraPos.x;
        playerY = cameraPos.y;
        playerZ = cameraPos.z;


        if (!camera.updateCamera(cameraPos, cameraUp))
        {
            return false;
        }

        return true;
    }
}