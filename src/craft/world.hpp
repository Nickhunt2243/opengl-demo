//
// Created by admin on 6/8/2024.
//

#ifndef OPENGLDEMO_WORLD_HPP
#define OPENGLDEMO_WORLD_HPP

#include <unordered_set>

#include "player.hpp"
#include "chunk.hpp"
#include "../setup/program.hpp"

namespace Craft
{
    class World
    {
    public:
        World(
            Engine::Window* window,
            Engine::Program* program,
            unsigned int width,
            unsigned int height
        );
        ~World();
        /**
         * A function for initializing the world.
         *
         * Setups the textures, initial chunks, etx.
         *
         * @return
         */
        bool initWorld();
        /**
         * A function for updating the world's state.
         *
         * @return True if the state was updated properly.
         */
        bool updateWorld();
        /**
         * A function for drawing the worlds scene.
         *
         * @return True if the world was drawn.
         */
        bool drawWorld();
    private:
        /// A pointer to the OpenGL program.
        Engine::Program* program;
        /// The unordered set of hashed coordinates.
        std::unordered_set<size_t> coords{};
        /// The camera object of the application.
        Player player;
        /// The cube shape to be rendered.
        std::vector<Chunk*> chunks;
        /// The pointer to the textures object, holding all information on the generated textures.
        Textures* textures{nullptr};
        /// The pool instance of the chunk.
        ThreadPool pool{std::thread::hardware_concurrency()};
        /// The array of futures to be ran through the thread pool.
        std::vector<std::future<void>> futures{};
        /// The mutex for the multi-threading.
        std::mutex mutex{};
    };
}

#endif //OPENGLDEMO_WORLD_HPP
