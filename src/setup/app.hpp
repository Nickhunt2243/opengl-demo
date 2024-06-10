#ifndef OPENGLDEMO_APP_HPP
#define OPENGLDEMO_APP_HPP

#include "window.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "../craft/player.hpp"
#include "../craft/chunk.hpp"
#include "../craft/textures.hpp"
#include "../craft/world.hpp"

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
        /// The Program object of the application
        Program program;
        /// The player world.
        Craft::World* world;
    };
}

#endif //OPENGLDEMO_APP_HPP
