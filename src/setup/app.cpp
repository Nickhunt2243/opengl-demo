#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "app.hpp"
#include "../helpers/helpers.hpp"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define VERT_SHADER_PATH "src/shader/default.vert"
#define FRAG_SHADER_PATH "src/shader/default.frag"

namespace Engine
{
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL")
        , program(VERT_SHADER_PATH, FRAG_SHADER_PATH)
        , camera{&window, &program}
        , cube{20}
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
        std::vector<float> vertices = cube.getVertices();
        std::vector<unsigned int> indices = cube.getIndices();

        std::cout << "Initializing Camera." << std::endl;
        if (!camera.initCamera())
        {
            return false;
        }
        std::cout << "Initializing View." << std::endl;
        initViewModel();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        bindVAO(indices.data(), (GLuint) indices.size(), vertices.data(), (GLuint) vertices.size());
        return true;
    }
    void Application::run()
    {
        std::cout << "Running..." << std::endl;
        GLint vertexColorLocation = glGetUniformLocation(program.getProgram(), "u_ourColor");
        while (!window.shouldClose())
        {
            // Process Input
            program.useProgram();
            camera.updateCamera();
            // Update State
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            auto timeValue = (float) glfwGetTime();
            float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
            float redValue = (cos(timeValue) / 2.0f) + 0.5f;
            float blueValue = (cos(timeValue) / 2.0f) + 0.5f;
            glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);
            // Display State
            draw();
            glfwSwapBuffers(window.getWindow());
            glfwPollEvents();
        }
    }
    void Application::bindVAO(GLuint* indices, GLuint indexCount, float* vertices, GLuint vertexCount)
    {
        GLuint aPosIndex = 0;
        // Generate Buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind vertex array
        glBindVertexArray(VAO);
        // Bind, enable, and initialize the VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(aPosIndex);
        glVertexAttribPointer(aPosIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        // Bind and initialize the EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
        // Unbind all buffers
        glBindVertexArray(0); // Unbind VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO
    }
    void Application::draw() const
    {
        int numIndices = (int) cube.getIndices().size();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
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