#ifndef OPENGLDEMO_WINDOW_HPP
#define OPENGLDEMO_WINDOW_HPP
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <string>

namespace Engine
{
    class Window
    {
    public:
        /**
         * Initialize the GLFW Window.
         *
         * @param width:  The width of the given window.
         * @param height: The height of the given window.
         * @param name:   The name of the given window.
         */
        explicit Window(int width, int height, std::string name);
        ~Window();
        /**
         * Retrieve whether the window should close.
         *
         * Use ctrl + c || esc to close window.
         *
         * @return true if the window else false.
         */
        bool shouldClose();
        /**
         * Retrieve the GLFW window pointer.
         *
         * @return A pointer to the GLFW window.
         */
        GLFWwindow* getWindow();
        /**
         * Initialize the Window.
         *
         * Initializes the GLFW window, adds window hints, create the window, adds callbacks for later use.
         *
         * @return true if successful else false.
         */
        bool initWindow();
    private:
        /**
         * Initialize GLAD, the OpenGL wrapper for this project.
         *
         * @return true if successful else false.
         */
        static bool initGLAD();
        /// A pointer to a GLFW window.
        GLFWwindow* window{};
        /// The width of the GLFW window.
        int width;
        /// The height of the GLFW window.
        int height;
        /// The name of the GLFW window.
        std::string name;
    };
}
#endif //OPENGLDEMO_WINDOW_HPP