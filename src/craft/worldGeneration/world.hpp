#ifndef OPENGLDEMO_WORLD_HPP
#define OPENGLDEMO_WORLD_HPP

#include <unordered_set>

#include "../../helpers/timer.hpp"
#include "../../helpers/helpers.hpp"
#include "../entities/player.hpp"
#include "chunk.hpp"
#include "../../setup/program.hpp"
#include "../../setup/compute.hpp"
#include "../weather/sun.hpp"

namespace Craft
{
    class World
    {
    public:
        World(
            Engine::Window* window,
            Engine::Program* blockProgram,
            Engine::Program* worldProgram,
            Engine::Compute* neighborCompute,
            uint32_t width,
            uint32_t height
        );
        ~World();
        /// The Window class that controls the GLFW lifecycle.
        Engine::Window* window;
        /// The camera object of the application.
        Player player;
        /// The mapping of 2D coordinates to Chunk*
        std::unordered_map<Coordinate2D<int>, std::unique_ptr<Chunk>> chunks{};
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
        /// The GLFW user pointer.
        GLFWUserPointer* userPointer;
        /// The mutex for the multi-threading.
        std::mutex chunkMutex{};
        /// The ID for the SSBO containing neighbor Information
        GLuint worldChunkInfoSSBOID;
        /**
         * A function for updating the surrounding blocks neighbor information when we place or delete a block.
         *
         * @param blockPos:      The position of the block being updated (placed/deleted).
         * @param chunkPos:      The position of the chunk being updated.
         * @param chunkInfoSSBO: The id of the SSBO containing neighbor information.
         * @param drawBlock:     A Boolean of whether we are drawing the given blocks sides.
         * @return:              A set of all the chunks that have been updated.
         */
        std::unordered_set<Coordinate2D<int>> updateNeighbors(
                Coordinate<int> blockPos,
                Coordinate2D<int> chunkPos,
                ChunkInfoSSBO* chunkInfoSSBO,
                bool drawBlock
        );
        ChunkInfoSSBO* chunkSSBOPointer{nullptr};
        /// The pointer to the textures object, holding all information on the generated textures.
        Textures* textures{nullptr};
        /// Boolean that denotes when we need to update neighbor information
        bool needsToUpdateNeighbors{true};
        /**
         * Run the compute shader to determine if a block is next to other blocks.
         *
         * If the block is next to other blocks, then we will not draw that side. Culling is fun!
         *
         * @param chunksAdded: The chunks to be updated.
         */
        void calcNeighborInfo();
    private:
        GLuint VAO{0},
               VBO{0};
        /// The game timer.
        Engine::Timer timer;
        /// A blockProgram for drawing blocks.
        Engine::Program* blockProgram;
        /// Generic blockProgram for drawing world objects.
        Engine::Program* worldProgram;
        /// A blockProgram for our neighbor information compute.
        Engine::Compute* neighborCompute;
        /// The pool instance of the chunk.
        ThreadPool pool{std::thread::hardware_concurrency()};
        /// The array of futures to be ran through the thread pool.
        std::vector<std::future<void>> futures{};
        /// A mutex for access the coords set.
        std::mutex coordsMutex{};
        std::mutex glMutex{};
        /// The chunks that we need to update neighbor information for.
        std::vector<Coordinate2D<int>> chunksAdded;
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
        std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*> coords{};
        /// The sun object for handling the in-game time.
        Sun sun;
        /**
         * A helper function for initializing a chunk.
         *
         * @param x: The x coordinate of the chunks center.
         * @param z: The z coordinate of the chunks center.
         */
        void initChunk(int x, int z, NeighborInfo* visibility);
        /// Calculate the new bounds of the chunks to render.
        void updateChunkBounds();
        /**
         * Update the chunks when a player moves to another chunk.
         *
         * This means initializing new chunks, calculating neighbor information, and deleting old chunks.
         *
         * @param directionDiff: The direction the player is moving.
         */
        void updateChunksLoaded(Coordinate2D<int> directionDiff);
        /**
         * Translate the neighbor information to account for players moving to new chunks.
         *
         * @param directionDiff: The direction the player is moving.
         */
        void translateChunkSSBOData(Coordinate2D<int> directionDiff) const;

        void initBuffers();
        GLint* blockCounts{nullptr};
        GLsizei* chunkFirst{nullptr};
    };
}

#endif //OPENGLDEMO_WORLD_HPP
