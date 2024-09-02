#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>

#include "app.hpp"
#include "../craft/misc/textures.hpp"

#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1000

#define BLOCK_VERT_SHADER_PATH "src/assets/shader/block.vert"
#define BLOCK_GEOM_SHADER_PATH "src/assets/shader/block.geom"
#define BLOCK_FRAG_SHADER_PATH "src/assets/shader/block.frag"
#define WORLD_VERT_SHADER_PATH "src/assets/shader/default.vert"
#define WORLD_GEOM_SHADER_PATH ""
#define WORLD_FRAG_SHADER_PATH "src/assets/shader/default.frag"
#define ORTHO_VERT_SHADER_PATH "src/assets/shader/ortho.vert"
#define ORTHO_GEOM_SHADER_PATH ""
#define ORTHO_FRAG_SHADER_PATH "src/assets/shader/ortho.frag"
#define SCENE_VERT_SHADER_PATH "src/assets/shader/scene.vert"
#define SCENE_GEOM_SHADER_PATH ""
#define SCENE_FRAG_SHADER_PATH "src/assets/shader/scene.frag"
#define NEIGHBOR_COMP_SHADER_PATH "src/assets/shader/neighbor.comp"

namespace Engine
{
    Application::Application()
        : window(WINDOW_WIDTH, WINDOW_HEIGHT, "ChunkCraft")
        , program{new Program(BLOCK_VERT_SHADER_PATH, BLOCK_GEOM_SHADER_PATH, BLOCK_FRAG_SHADER_PATH)}
        , worldProgram{new Program(WORLD_VERT_SHADER_PATH, WORLD_GEOM_SHADER_PATH, WORLD_FRAG_SHADER_PATH)}
        , orthoProgram{new Program(ORTHO_VERT_SHADER_PATH, ORTHO_GEOM_SHADER_PATH, ORTHO_FRAG_SHADER_PATH)}
        , sceneProgram{new Program(SCENE_VERT_SHADER_PATH, SCENE_GEOM_SHADER_PATH, SCENE_FRAG_SHADER_PATH)}
        , neighborCompute{new Compute(NEIGHBOR_COMP_SHADER_PATH)}
        , world{new Craft::World(&window, program, worldProgram, neighborCompute, WINDOW_WIDTH, WINDOW_HEIGHT)}
        , crossHair{new Craft::CrossHair(orthoProgram, &window)}
    {}
    Application::~Application()
    {
        delete crossHair;
        delete world;
        delete neighborCompute;
        delete worldProgram;
        delete orthoProgram;
        delete program;
        glDeleteFramebuffers(1, &FBO);
    }
    void Application::initFBO()
    {
        int width = window.getWidth();
        int height = window.getHeight();

        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // Create texture to hold the color attachment
        glGenTextures(1, &frameTexture);
        glBindTexture(GL_TEXTURE_2D, frameTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTexture, 0);

        // Create a renderbuffer for depth and stencil attachment (optional)
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    void Application::initQuad()
    {
        sceneProgram->useProgram();
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (quadVertices.size() * sizeof(float)), quadVertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
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
        if (!orthoProgram->initProgram())
        {
            std::cerr << "Failed to initialize ortho program." << std::endl;
            return false;
        }
        if (!sceneProgram->initProgram())
        {
            std::cerr << "Failed to initialize scene program." << std::endl;
            return false;
        }
        if (!neighborCompute->initCompute())
        {
            std::cerr << "Failed to initialize neighbors compute." << std::endl;
            return false;
        }

        // Initialize the ProjT.
        projMatrix = glm::perspective(glm::radians(80.0f), ((float) WINDOW_WIDTH /  (float) WINDOW_HEIGHT), 0.1f, 4000.0f);

        initFBO();
        initQuad();
        world->initWorld();
        crossHair->initCrossHair();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        return true;
    }
    void Application::updateScene()
    {
        program->useProgram();
        setMat4(program->getProgram(), "u_projT", projMatrix);
        worldProgram->useProgram();
        setMat4(worldProgram->getProgram(), "u_projT", projMatrix);

        world->updateWorld();
    }
    void Application::drawScene()
    {
        /// Draw World
        world->drawWorld();
    }
    void Application::drawHUD()
    {
        crossHair->drawCrossHair(frameTexture);
    }
    void Application::drawQuad()
    {
        sceneProgram->useProgram();
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(sceneProgram->getProgram(), "screenTexture"), 0);
        glBindTexture(GL_TEXTURE_2D, frameTexture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    void Application::run()
    {
        std::cout << "Running..." << std::endl;
        while (!window.shouldClose())
        {
            // draw to the frame buffer.
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            updateScene();
            drawScene();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Draw the frame buffer to screen and use it for hud drawing.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Draw the quad of the frame that was generated from the drawScene function.
            drawQuad();
            // Draw the crossHair last as it uses special proj and view matrices.
            drawHUD();
            glfwSwapBuffers(window.getWindow());
            // Poll for mouse and keyboard events
            glfwPollEvents();

        }
    }
}