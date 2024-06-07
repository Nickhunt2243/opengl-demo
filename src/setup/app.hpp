#ifndef OPENGLDEMO_APP_HPP
#define OPENGLDEMO_APP_HPP

#include "window.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "../craft/chunk.hpp"
#include "../craft/textures.hpp"
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
        std::vector<Craft::Chunk*> chunks;
        /// The pointer to the textures object, holding all information on the generated textures.
        Craft::Textures* textures{nullptr};
        /// The unordered set of hashed coordinates.
        std::unordered_set<size_t> coords{};
    };
}

#endif //OPENGLDEMO_APP_HPP
