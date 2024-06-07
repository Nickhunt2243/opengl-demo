#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <thread>
#include <mutex>

#include "app.hpp"
#include "../craft/textures.hpp"

#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1000
#define VERT_SHADER_PATH "src/assets/shader/default.vert"
#define GEOM_SHADER_PATH "src/assets/shader/default.geom"
#define FRAG_SHADER_PATH "src/assets/shader/default.frag"
#define CHUNK_WIDTH 2
#define NUM_CHUNKS (CHUNK_WIDTH + CHUNK_WIDTH + 1) * (CHUNK_WIDTH + CHUNK_WIDTH + 1) /// 5 x 5

namespace Engine
{
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL")
        , program(VERT_SHADER_PATH, GEOM_SHADER_PATH, FRAG_SHADER_PATH)
        , camera{&window, &program, WINDOW_WIDTH, WINDOW_HEIGHT}
    {
        for (int i = 0; i<NUM_CHUNKS; i++)
        {
            chunks.push_back(new Craft::Chunk(&coords));
        }
    }
    Application::~Application() {
        for (int i = 0; i<NUM_CHUNKS; i++)
        {
            delete chunks[i];
        }
        delete textures;
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
        // init world
        textures = new Craft::Textures();
        textures->initTextures(program.getProgram());
        Craft::Chunk::textures = textures;

        int idx = 0;
        for (int i = -CHUNK_WIDTH; i<CHUNK_WIDTH+1; i++)
        {
            for (int j = -CHUNK_WIDTH; j<CHUNK_WIDTH+1; j++)
            {
                chunks[idx]->initChunk(i, j);
                idx++;
            }
        }
        for (int i = 0; i<NUM_CHUNKS; i++)
        {
            chunks[i]->findNeighbors();
            chunks[i]->initBuffers();
        }

        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera())
        {
            return false;
        }

        glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
        return true;
    }
    void Application::run()
    {
        std::cout << "Running..." << std::endl;
        while (!window.shouldClose())
        {
            // Process Input
            if (!camera.updateCamera()) {
                return;
            }

            // Update State
            glClearColor(0.7f, 0.7f, 0.9f, 1.0f);
            // Clear the color and depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Display State
            for (int i=0; i<NUM_CHUNKS; i++) {
                chunks[i]->drawChunk();
            }
            glfwSwapBuffers(window.getWindow());
            glfwPollEvents();
        }
    }
}