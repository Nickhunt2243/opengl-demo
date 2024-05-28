#ifndef OPENGLDEMO_APP_HPP
#define OPENGLDEMO_APP_HPP

#include "window.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "../shapes/cube.hpp."
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
        /// The camera object of the application.
        Camera camera;
        /// The cube shape to be rendered.
        Craft::World world;
        /// The Vertex Array Object of the OpenGL program.
        GLuint VAO{0},
        /// The Vertex Buffer Object of the OpenGL program.
               VBO{0};
    };
}

#endif //OPENGLDEMO_APP_HPP
