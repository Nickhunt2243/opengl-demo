#ifndef OPENGLDEMO_APP_HPP
#define OPENGLDEMO_APP_HPP

#include "window.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "../craft/entities/player.hpp"
#include "../craft/worldGeneration/chunk.hpp"
#include "../craft/misc/textures.hpp"
#include "../craft/worldGeneration/world.hpp"

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
        /// The player world.
        Craft::World* world;
    };
}

#endif //OPENGLDEMO_APP_HPP
