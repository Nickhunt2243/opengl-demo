

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "camera.hpp"
#include "../helpers/helpers.hpp"

namespace Engine
{
    bool Camera::firstMouse = true;
    float Camera::lastX = 0.0f;
    float Camera::lastY = 0.0f;
    float Camera::yaw = 180.0f;
    float Camera::pitch = -20.0f;

    Camera::Camera(
            Window* window,
            Engine::Program* program,
            Engine::Program* worldProgram,
            uint32_t width,
            uint32_t height,
            float x, float y, float z
    )
        : window(window)
        , program(program)
        , worldProgram(worldProgram)
        , windowWidth(width)
        , windowHeight(height)
    {}

    bool Camera::initCamera()
    {
        if (window == nullptr || program == nullptr)
        {
            std::cerr << "Window or Program not initialized" << std::endl;
            return false;
        }
        // Will eventually clean up how I handle all the programs. But for now there are two, so I am not going to panic.
        program->useProgram();
        setMat4(program->getProgram(), "u_viewT", view);
        worldProgram->useProgram();
        setMat4(worldProgram->getProgram(), "u_viewT", view);

        int width, height;
        glfwGetWindowSize(window->getWindow(), &width, &height);
        Camera::lastX = static_cast<float>(width) / 2.0f;
        Camera::lastY = static_cast<float>(height) / 2.0f;
        glfwSetWindowUserPointer(window->getWindow(), this);
        glfwSetCursorPosCallback(window->getWindow(), mouse_movement_callback);

        return true;
    }

    bool Camera::updateCamera(glm::vec3 cameraPos, glm::vec3 cameraUp)
    {

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        program->useProgram();
        if (!setMat4(program->getProgram(), "u_viewT", view)) {
            return false;
        }

        worldProgram->useProgram();
        if (!setMat4(worldProgram->getProgram(), "u_viewT", view)) {
            return false;
        }
        return true;
    }

    void Camera::mouse_movement_callback(GLFWwindow* window, double xPos, double yPos)
    {
        auto userPointerData = static_cast<Craft::GLFWUserPointer*>(glfwGetWindowUserPointer(window));
        Camera* camera = userPointerData->camera;
        if (Camera::firstMouse)
        {
            Camera::lastX = static_cast<float>(xPos);
            Camera::lastY = static_cast<float>(yPos);
            Camera::firstMouse = false;
        }

        float xoffset = static_cast<float>(xPos) - Camera::lastX;
        float yoffset = Camera::lastY - static_cast<float>(yPos);
        Camera::lastX = static_cast<float>(xPos);
        Camera::lastY = static_cast<float>(yPos);

        xoffset *= camera->sensitivity;
        yoffset *= camera->sensitivity;

        Camera::yaw += xoffset;
        Camera::pitch += yoffset;

        if (Camera::pitch > 89.0f)
        {
            Camera::pitch = 89.0f;
        }
        if (Camera::pitch < -89.0f)
        {
            Camera::pitch = -89.0f;
        }

        glm::vec3 direction;
        direction.x = cos(glm::radians(Camera::yaw)) * cos(glm::radians(Camera::pitch));
        direction.y = sin(glm::radians(Camera::pitch));
        direction.z = sin(glm::radians(Camera::yaw)) * cos(glm::radians(Camera::pitch));
        camera->cameraFront = glm::normalize(direction);
    }

    glm::vec3 Camera::getCameraFront()
    {
        return cameraFront;
    }
}
