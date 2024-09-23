#ifndef OPENGLDEMO_APP_HPP
#define OPENGLDEMO_APP_HPP

#include "window.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "compute.hpp"
#include "../craft/entities/player.hpp"
#include "../craft/worldGeneration/chunk.hpp"
#include "../craft/misc/textures.hpp"
#include "../craft/worldGeneration/world.hpp"
#include "../craft/misc/crossHair.hpp"

namespace Engine
{
    class Application
    {
    public:
        Application();
        ~Application();
        /**
         * Initialize the Applications GLFW window, OpenGL Program, Buffers, and Camera.
         *
         * @return true if successful else false.
         */
        bool initialize();
        /**
         * Run the Application.
         *
         * Use ctrl + c or esc to close the application
         */
        void run();
    private:
        /// The Window object of the application.
        Window window;
        /// The Program for blocks within the application
        Program* program;
        /// The program for generic models within the application.
        Program* worldProgram;
        /// The program for rendering 2D objects for the HUD.
        Program* orthoProgram;
        /// The program for our neighbor compute
        Compute* neighborCompute;
        /// The program for our neighbor compute
        Compute* ambientOccCompute;
        /// The program for rendering the scenes quad to the screen.
        Program* sceneProgram;
        /// The player world.
        Craft::World* world;
        /// The game crossHair.
        Craft::CrossHair* crossHair;

        std::vector<float> quadVertices = {
                1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,

                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f,  1.0f, 1.0f, 1.0f
        };

        GLuint FBO, RBO, frameTexture, quadVAO, quadVBO;
        glm::mat4 projMatrix{1.0f};

        void initFBO();
        void initQuad();
        void updateScene();
        void drawScene();
        void drawHUD();
        void drawQuad();
    };
}

#endif //OPENGLDEMO_APP_HPP
