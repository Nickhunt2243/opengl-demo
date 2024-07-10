#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>

#include "app.hpp"
#include "../craft/misc/textures.hpp"

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 920
#define BLOCK_VERT_SHADER_PATH "src/assets/shader/block.vert"
#define BLOCK_GEOM_SHADER_PATH "src/assets/shader/block.geom"
#define BLOCK_FRAG_SHADER_PATH "src/assets/shader/block.frag"

#define WORLD_VERT_SHADER_PATH "src/assets/shader/default.vert"
#define WORLD_GEOM_SHADER_PATH ""
#define WORLD_FRAG_SHADER_PATH "src/assets/shader/default.frag"


namespace Engine
{
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "ChunkCraft")
        , program{new Program(BLOCK_VERT_SHADER_PATH, BLOCK_GEOM_SHADER_PATH, BLOCK_FRAG_SHADER_PATH)}
        , worldProgram{new Program(WORLD_VERT_SHADER_PATH, WORLD_GEOM_SHADER_PATH, WORLD_FRAG_SHADER_PATH)}
        , world{new Craft::World(&window, program, worldProgram, WINDOW_WIDTH, WINDOW_HEIGHT)}
    {}
    Application::~Application()
    {
        delete world;
        delete worldProgram;
        delete program;
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
        if (!program->initProgram())
        {
            std::cerr << "Failed to initialize program." << std::endl;
            return false;
        }
        if (!worldProgram->initProgram())
        {
            std::cerr << "Failed to initialize world program." << std::endl;
            return false;
        }

        // Initialize the ProjT.
        glm::mat4 projMatrix = glm::perspective(glm::radians(70.0f), ((float) WINDOW_WIDTH /  (float) WINDOW_HEIGHT), 0.1f, 4000.0f);
        program->useProgram();
        setMat4(program->getProgram(), "u_projT", projMatrix);
        worldProgram->useProgram();
        setMat4(worldProgram->getProgram(), "u_projT", projMatrix);

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
//            glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
            // Clear the color and depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            /// Draw World
            world->drawWorld();
            glfwSwapBuffers(window.getWindow());
            glfwPollEvents();
        }
    }
}