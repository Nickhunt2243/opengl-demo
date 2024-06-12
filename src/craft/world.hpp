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
        Coordinate2D playerOriginCoord;
        /// The mapping of 2D coordinates to Chunk*
        std::unordered_map<Coordinate2D, Chunk*> chunks{};
        /// The pointer to the textures object, holding all information on the generated textures.
        Textures* textures{nullptr};
        /// The pool instance of the chunk.
        ThreadPool pool{std::thread::hardware_concurrency()};
        /// The array of futures to be ran through the thread pool.
        std::vector<std::future<void>> futures{};
        /// The mutex for the multi-threading.
        std::mutex chunkMutex{};
        /// A mutex for access the coords set.
        std::mutex coordsMutex{};
        /// The X coordinate of the first chunk to render (inclusive).
        int chunkStartX;
        /// The Z coordinate of the first chunk to render (inclusive).
        int chunkStartZ;
        /// The X coordinate of the last chunk to render (exclusive).
        int chunkEndX;
        /// The Z coordinate of the last chunk to render (exclusive).
        int chunkEndZ;
        /**
         * A helper function for initializing a chunk.
         *
         * @param x: The x coordinate of the chunks center.
         * @param z: The z coordinate of the chunks center.
         */
        void initChunk(int x, int z);
        /// Calculate the new bounds of the chunks to render.
        void calcChunkBounds();
    };
}

#endif //OPENGLDEMO_WORLD_HPP
