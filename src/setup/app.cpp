#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "app.hpp"
#include "../craft/block.hpp"
#include "../helpers/helpers.hpp"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define VERT_SHADER_PATH "src/shader/default.vert"
#define GEOM_SHADER_PATH "src/shader/default.geom"
#define FRAG_SHADER_PATH "src/shader/default.frag"
#define TOP_TEX_PATH  "src/assets/textures/grass_block/grass_block_top.png"
#define BOTTOM_TEX_PATH  "src/assets/textures/grass_block/dirt.png"
#define SIDE_TEX_PATH  "src/assets/textures/grass_block/grass_block_side.png"



namespace Engine
{
    std::unordered_map<std::string, std::string> Application::texMap {
        {"grass_top", TOP_TEX_PATH},
        {"grass_bottom", BOTTOM_TEX_PATH},
        {"grass_side", SIDE_TEX_PATH}
    };
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL")
        , program(VERT_SHADER_PATH, GEOM_SHADER_PATH, FRAG_SHADER_PATH, Application::texMap)
        , camera{&window, &program}
        , world()
    {}
    Application::~Application()
    {
        glDeleteVertexArrays(1, &(VAO));
        glDeleteBuffers(1, &(VBO));
        glDeleteBuffers(1, &(EBO));
    }
    bool Application::initialize()
    {
        std::cout << "Initializing Window." << std::endl;
        if (!window.initWindow())
        {
            return false;
        }
        std::cout << "Initializing Program." << std::endl;
        if (!program.initProgram())
        {
            return false;
        }
        program.useProgram();
        // Init VAO and VBO
        glGenVertexArrays(1, &VAO);
        glCreateBuffers(1, &VBO);
        // init world
        world.initializeWorld(VAO, VBO);

        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera())
        {
            return false;
        }
        std::cout << "Initializing View." << std::endl;
        initViewModel();

        glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);

        std::vector<float>vertices = {
                -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, -1.0f,
                0.0f, 0.0f, -1.0f,
                1.0f, 0.0f, -1.0f,
                -1.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 1.0f,
        };
//        bindVAO(vertices.data(), (GLuint) vertices.size());
        return true;
    }
    void Application::run()
    {
        std::cout << "Running..." << std::endl;
        GLint vertexColorLocation = glGetUniformLocation(program.getProgram(), "u_ourColor");
        program.useProgram();
        while (!window.shouldClose())
        {
            // Process Input
            if (!camera.updateCamera()) {
                return;
            }
            // Update State
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            // Clear the color and depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Display State
            world.drawWorld();
            glfwSwapBuffers(window.getWindow());
            glfwPollEvents();
        }
    }
    void printMatrix4x4(glm::mat4& mat, const std::string& name)
    {
        std::cout << "\n" << name << " Matrix:" << std::endl;
        std::cout << "{\n\t";
        glm::mat4 trans = glm::transpose(mat);
        for (int i=0; i<16;i++)
        {
            std::cout << glm::value_ptr(trans)[i] << " ";
            if (i % 4 == 3 && i != 15) std::cout << "\n\t";
        }
        std::cout << "\n}" << std::endl;
    }
    void Application::initViewModel()
    {
        // Projection Matrix
        glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 30.0f);
        // Model Matrix (for demo)
        glm::mat4 modelMatrix{1.0f};
        modelMatrix = glm::scale(modelMatrix, glm::vec3{1.0f, 1.0f, 1.0f});

        Helpers::setMat4(program.getProgram(), "u_projT", projMatrix);
        Helpers::setMat4(program.getProgram(), "u_modelT", modelMatrix);
    }

}