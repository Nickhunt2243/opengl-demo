//
// Created by admin on 6/8/2024.
//

#ifndef OPENGLDEMO_WORLD_HPP
#define OPENGLDEMO_WORLD_HPP

#include <unordered_set>

#include "../../helpers/timer.hpp"
#include "../entities/player.hpp"
#include "chunk.hpp"
#include "../../setup/program.hpp"
#include "../weather/sun.hpp"

namespace Craft
{
    class World
    {
    public:
        World(
            Engine::Window* window,
            Engine::Program* program,
            Engine::Program* worldProgram,
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
        /// The game timer.
        Engine::Timer timer;
        /// A program for drawing blocks.
        Engine::Program* program;
        /// Generic program for drawing world objects.
        Engine::Program* worldProgram;
        /// The camera object of the application.
        Player player;
        /// The mapping of 2D coordinates to Chunk*
        std::unordered_map<Coordinate2D<int>, Chunk*> chunks{};
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
        /// A Coordinate 2D for catching chunk difference failure and camera update failures.
        Coordinate2D<int> failureCoord{-2, -2};
        /// A mapping of chunk coords to a block position bitmap.
        std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*> coords{};
        /// The mutex for only allowing us to create one set of chunks at a time.
        std::mutex initOneSideChunksMutex{};
        /// The sun object for handling the in-game time.
        Sun sun;
        /**
         * A helper function for initializing a chunk.
         *
         * @param x: The x coordinate of the chunks center.
         * @param z: The z coordinate of the chunks center.
         */
        void initChunk(int x, int z);
        /// Calculate the new bounds of the chunks to render.
        void updateChunkBounds();
    };
}

#endif //OPENGLDEMO_WORLD_HPP
