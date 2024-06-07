#include <iostream>

#include "window.hpp"

namespace Engine
{
    Window::Window(int width, int height, std::string name)
        : width( width )
        , height( height )
        , name( std::move(name) )
    {}
    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    bool Window::shouldClose()
    {
        return glfwWindowShouldClose(window);
    }
    GLFWwindow* Window::getWindow()
    {
        return window;
    }
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if ((key == GLFW_KEY_C && (mods & GLFW_MOD_CONTROL) ) || (key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
    bool Window::initWindow()
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        if (window == nullptr)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);
        if (!initGLAD()) return false;
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetKeyCallback(window, key_callback);
        return true;
    }
    bool Window::initGLAD()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        return true;
    }
}