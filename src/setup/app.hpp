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
               VBO{0},
        /// The Element Buffer Object of the OpenGL program.
               EBO{0};
        static std::unordered_map<std::string, std::string> texMap;
        /**
         * Bind the vertices and indices to the buffers to be sent to the shaders for rendering.
         *
         * @param indices:     The pointer to the array of indices. (Not currently used due to geometry shader).
         * @param indexCount:  The number of indices. (Not currently used due to geometry shader).
         * @param vertices:    The pointer to the array of vertices.
         * @param vertexCount: The number of vertices.
         */
        void bindVAO(float* vertices, GLuint vertexCount);
        /// Bind the VAO and draw the elements.
        void draw() const;
        /// Construct the 4x4 Projection matrix and 4x4 Model matrix and send them to the shaders for rendering.
        void initViewModel();
    };
}

#endif //OPENGLDEMO_APP_HPP
