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
            std::unordered_set<size_t>* coords
    )
        : coords{coords}
        , playerX{0.0f}
        , playerY{5.0f}
        , playerZ{0.0f}
        , window{window}
        , camera{window, program, width, height, playerX, playerY, playerZ}
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
        float roundedX = round(playerX),
              roundedY = round(playerY),
              roundedZ = round(playerZ);

//        size_t blockBelow = Coordinate::compute_hash(roundedX, roundedY-2.0f, roundedZ);
        Coordinate playerCoord{roundedX, roundedY, roundedZ};
        size_t altCoord = playerCoord.add(0.0f, -playerHeight, 0.0f);
        return coords->find(altCoord) != coords->end();
    }
    bool Player::updatePlayer()
    {
        glm::vec3 cameraPos = glm::vec3{playerX, playerY, playerZ};
        glm::vec3 cameraFront = camera.getCameraFront();
        if (glfwGetKey(window->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos += cameraWalkingSpeed * glm::vec3{cameraFront.x, 0.0, cameraFront.z};
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos -= cameraWalkingSpeed * glm::vec3{cameraFront.x, 0.0, cameraFront.z};
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraWalkingSpeed;
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraWalkingSpeed;
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * cameraWalkingSpeed;
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            cameraPos += glm::vec3(0.0f, -1.0f, 0.0f) * cameraWalkingSpeed;
        }

        bool stopFall = blockBelowPlayer();

        if (isFalling && stopFall)
        {
            isFalling = false;
        }
        else if (isFalling && !stopFall)
        {
            cameraPos.y -= fallingSpeed;
        }
        else if (!isFalling && !stopFall)
        {
            isFalling = true;
        }

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