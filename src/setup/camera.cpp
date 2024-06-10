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
    float Camera::yaw = -90.0f;
    float Camera::pitch = 0.0f;

    Camera::Camera(
            Window* window,
            Program* program,
            unsigned int width,
            unsigned int height,
            float x, float y, float z
    )
        : window(window)
        , program(program)
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

        // Initialize the Proj and Model matrices. These do not change.
        glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), (float) windowWidth / (float) windowHeight, 0.1f, 100.0f);
        setMat4(program->getProgram(), "u_projT", projMatrix);
        setMat4(program->getProgram(), "u_viewT", view);
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
        if (!setMat4(program->getProgram(), "u_viewT", view)) {
            return false;
        }
        return true;
    }

    void Camera::mouse_movement_callback(GLFWwindow* window, double xPos, double yPos)
    {
        auto camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
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
//    glm::vec3 Camera::getCameraUp()
//    {
//        return cameraUp;
//    }
}
