#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>

#include "app.hpp"
#include "../craft/textures.hpp"

#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1000
#define VERT_SHADER_PATH "src/assets/shader/default.vert"
#define GEOM_SHADER_PATH "src/assets/shader/default.geom"
#define FRAG_SHADER_PATH "src/assets/shader/default.frag"

namespace Engine
{
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL")
        , program(VERT_SHADER_PATH, GEOM_SHADER_PATH, FRAG_SHADER_PATH)
        , world{new Craft::World(&window, &program, WINDOW_WIDTH, WINDOW_HEIGHT)}
    {

    }
    Application::~Application() {
        delete world;
    }
    bool Application::initialize()
    {
        std::cout << "Initializing Window." << std::endl;
        if (!window.initWindow())
        {
            std::cerr << "Failed to initialize window." << std::endl;
            return false;
        }
        std::cout << "Initializing Program." << std::endl;
        if (!program.initProgram())
        {
            std::cerr << "Failed to initialize program." << std::endl;
            return false;
        }

        program.useProgram();
        world->initWorld();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        return true;
    }
    void Application::run()
    {
        std::cout << "Running..." << std::endl;
        while (!window.shouldClose())
        {
            // Process Input
            world->updateWorld();
            // Update State
            glClearColor(0.7f, 0.7f, 0.9f, 1.0f);
            // Clear the color and depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            /// Draw World
            world->drawWorld();
            glfwSwapBuffers(window.getWindow());
            glfwPollEvents();
        }
    }
}