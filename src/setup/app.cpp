#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "app.hpp"
#include "../craft/textures.hpp"

#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1000
#define VERT_SHADER_PATH "src/shader/default.vert"
#define GEOM_SHADER_PATH "src/shader/default.geom"
#define FRAG_SHADER_PATH "src/shader/default.frag"


namespace Engine
{
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL")
        , program(VERT_SHADER_PATH, GEOM_SHADER_PATH, FRAG_SHADER_PATH)
        , camera{&window, &program, WINDOW_WIDTH, WINDOW_HEIGHT}
        , world()
    {}
    Application::~Application()
    {
        glDeleteVertexArrays(1, &(VAO));
        glDeleteBuffers(1, &(VBO));
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

        // Init VAO and VBO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glEnableVertexAttribArray(0);
        // init world
        world.initWorld(program.getProgram());
        world.findNeighbors();
        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera())
        {
            return false;
        }

        glUniform1i(glGetUniformLocation(program.getProgram(), "textures[0]"), 0);
        glUniform1i(glGetUniformLocation(program.getProgram(), "textures[1]"), 1);
        glUniform1i(glGetUniformLocation(program.getProgram(), "textures[2]"), 2);
        glUniform1i(glGetUniformLocation(program.getProgram(), "textures[3]"), 3);
        glUniform1i(glGetUniformLocation(program.getProgram(), "textures[4]"), 4);
        glUniform1i(glGetUniformLocation(program.getProgram(), "textures[5]"), 5);

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
            glClearColor(0.5f, 0.5f, 0.7f, 1.0f);
            // Clear the color and depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Display State
            if (!world.drawWorld()) {
                std::cerr << "Failed to draw scene." << std::endl;
                return;
            }
            glfwSwapBuffers(window.getWindow());
            glfwPollEvents();
        }
    }
}