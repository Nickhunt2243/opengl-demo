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

    Camera::Camera(Window* window, Program* program)
            : window(window)
            , program(program)
    {}

    Camera::~Camera() = default;

    bool Camera::initCamera()
    {
        if (window == nullptr || program == nullptr)
        {
            std::cerr << "Window or Program not initialized" << std::endl;
            return false;
        }

        Helpers::setMat4(program->getProgram(), "u_viewT", view);
        int width, height;
        glfwGetWindowSize(window->getWindow(), &width, &height);
        Camera::lastX = static_cast<float>(width) / 2.0f;
        Camera::lastY = static_cast<float>(height) / 2.0f;
        glfwSetWindowUserPointer(window->getWindow(), this);
        glfwSetCursorPosCallback(window->getWindow(), mouse_movement_callback);

        return true;
    }

    void Camera::updateCamera()
    {
        if (glfwGetKey(window->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos += cameraWalkingSpeed * cameraFront;
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos -= cameraWalkingSpeed * cameraFront;
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraWalkingSpeed;
        }
        if (glfwGetKey(window->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraWalkingSpeed;
        }
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        Helpers::setMat4(program->getProgram(), "u_viewT", view);
    }

    void Camera::mouse_movement_callback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (Camera::firstMouse)
        {
            Camera::lastX = static_cast<float>(xpos);
            Camera::lastY = static_cast<float>(ypos);
            Camera::firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - Camera::lastX;
        float yoffset = Camera::lastY - static_cast<float>(ypos);
        Camera::lastX = static_cast<float>(xpos);
        Camera::lastY = static_cast<float>(ypos);

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

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
}
