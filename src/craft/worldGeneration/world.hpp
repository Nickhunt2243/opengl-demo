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
            Engine::Compute* ambientOccCompute,
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
        /// Vectors for descibing when we need to update certain aspects of a chunk.
        std::vector<Coordinate2D<int>> chunksToUpdateNeighborInfo{};
        std::vector<Coordinate2D<int>> chunksToUpdateAmbientInfo{};
        std::vector<Coordinate2D<int>> chunksToUpdateVBOInfo{};
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
        /// A pointer to the block SSBO.
        NeighborInfo* blockSSBOPointer{nullptr};
        /// A pointer to the chunk SSBO.
        Coordinate2D<int>* chunkSSBOPointer{nullptr};
        /// A pointer to the draw commands.
        DrawArraysIndirectCommand * drawCommandBufferPointer{nullptr};
        /// The pointer to the textures object, holding all information on the generated textures.
        Textures* textures{nullptr};
        /**
         * Run the compute shader to determine if a block is next to other blocks.
         *
         * If the block is next to other blocks, then we will not draw that side. Culling is fun!
         */
        void calcNeighborInfo();
        /**
         * Calculate a blocks ambient occlusion. Super simple voxel based ambient occlusion.
         *
         * Article: https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
         */
        void calcAmbientOcclusionInfo();
        /**
         * Update the blocks instance data.
         *
         * The instances to draw are based on each side of a block. If we group sides together, then we can
         * easily omit a side when we want to cull it.
         */
        void updateInstanceIdxVBO();
        /**
         * Given a blocks info (chunkPos and chunk relative block position), append another chunk if the block
         * is on the edge of the chunk.
         *
         * @param info: The info of the block being updated.
         */
        void appendAdditionalAffectedChunks(BlockInfo info);
        /**
         * Given a blocks info (chunkPos and chunk relative block position), explicitly update the neighbor information
         * to draw specific sides. This ensures that when you delete a block, the correct sides will be drawn
         * the same frame.
         *
         * @param info: The info of the block being updated.
         */
        void updateNeighbors(BlockInfo info);
    private:
        /// The Buffers and Array Objects.
        GLuint VAO{0};
        GLuint VBO{0};
        GLuint InstanceVBO{0};
        GLuint blockSSBO{0};
        GLuint chunkSSBO{0};
        GLuint idxSSBO{0};
        GLuint indirectBO{0};
        /// A pointer to the idx SSBO - Holds information on which idx within the blockSSBO a given instance pertains to.
        int* idxSSBOPointer{nullptr};
        /// The game timer.
        Engine::Timer timer;
        /// A blockProgram for drawing blocks.
        Engine::Program* blockProgram;
        /// Generic blockProgram for drawing world objects.
        Engine::Program* worldProgram;
        /// A blockProgram for our neighbor information compute.
        Engine::Compute* neighborCompute;
        /// A blockProgram for our neighbor information compute.
        Engine::Compute* ambientOccCompute;
        /// The pool instance of the chunk.
        ThreadPool pool{std::thread::hardware_concurrency()};
        /// The array of futures to be ran through the thread pool.
        std::vector<std::future<void>> futures{};
        /// A mutex for access the coords set.
        std::mutex coordsMutex{};
        /// Mutexes for updating the update vectors of Neighbor, Ambient Occ, and Instance info
        std::mutex chunkNeighborMutex{};
        std::mutex chunkAmbientMutex{};
        std::mutex chunkVBOMutex{};
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
        void initChunk(Coordinate2D<int> chunkPos);
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
        /// Initialize and map the necessary buffers.
        void initBuffers();
        GLsizei* instanceCount{nullptr};
        // 3 - pos
        // 3 - Norm
        // 2 - UV
        // 1 - Texture Offset
        // 1 - SideVisibility
        // 2 - Mask AmbientMask
        const std::vector<int> blockData = {
            // Y_max
                0, 1, 1, 0, 1, 0, 1, 0, 8, 2, 0, 2,
                1, 1, 1, 0, 1, 0, 1, 1, 8, 2, 0, 6,
                0, 1, 0, 0, 1, 0, 0, 0, 8, 2, 0, 0,
                1, 1, 0, 0, 1, 0, 0, 1, 8, 2, 0, 4,
                0, 1, 0, 0, 1, 0, 0, 0, 8, 2, 0, 0,
                1, 1, 1, 0, 1, 0, 1, 1, 8, 2, 0, 6,
            // Y_min
                1, 0, 1, 0, -1, 0, 0, 0, 11, 4, 0, 14,
                0, 0, 1, 0, -1, 0, 0, 1, 11, 4, 0, 10,
                1, 0, 0, 0, -1, 0, 1, 0, 11, 4, 0, 12,
                0, 0, 0, 0, -1, 0, 1, 1, 11, 4, 0, 8,
                1, 0, 0, 0, -1, 0, 1, 0, 11, 4, 0, 12,
                0, 0, 1, 0, -1, 0, 0, 1, 11, 4, 0, 10,
            // X_max
                1, 1, 1, 1, 0, 0, 0, 0, 14, 8, 1, 6,
                1, 0, 1, 1, 0, 0, 0, 1, 14, 8, 1, 2,
                1, 1, 0, 1, 0, 0, 1, 0, 14, 8, 1, 4,
                1, 0, 0, 1, 0, 0, 1, 1, 14, 8, 1, 0,
                1, 1, 0, 1, 0, 0, 1, 0, 14, 8, 1, 4,
                1, 0, 1, 1, 0, 0, 0, 1, 14, 8, 1, 2,
            // X_min
                0, 1, 0, -1, 0, 0, 0, 0, 17, 16, 1, 12,
                0, 0, 0, -1, 0, 0, 0, 1, 17, 16, 1, 8,
                0, 1, 1, -1, 0, 0, 1, 0, 17, 16, 1, 14,
                0, 0, 1, -1, 0, 0, 1, 1, 17, 16, 1, 10,
                0, 1, 1, -1, 0, 0, 1, 0, 17, 16, 1, 14,
                0, 0, 0, -1, 0, 0, 0, 1, 17, 16, 1, 8,
            // Z_max
                0, 1, 1, 0, 0, 1, 0, 0, 20, 32, 2, 4,
                0, 0, 1, 0, 0, 1, 0, 1, 20, 32, 2, 0,
                1, 1, 1, 0, 0, 1, 1, 0, 20, 32, 2, 6,
                1, 0, 1, 0, 0, 1, 1, 1, 20, 32, 2, 2,
                1, 1, 1, 0, 0, 1, 1, 0, 20, 32, 2, 6,
                0, 0, 1, 0, 0, 1, 0, 1, 20, 32, 2, 0,
            // Z_min
                1, 1, 0, 0, 0, -1, 0, 0, 23, 64, 2, 14,
                1, 0, 0, 0, 0, -1, 0, 1, 23, 64, 2, 10,
                0, 1, 0, 0, 0, -1, 1, 0, 23, 64, 2, 12,
                0, 0, 0, 0, 0, -1, 1, 1, 23, 64, 2, 8,
                0, 1, 0, 0, 0, -1, 1, 0, 23, 64, 2, 12,
                1, 0, 0, 0, 0, -1, 0, 1, 23, 64, 2, 10
        };
    };
}

#endif //OPENGLDEMO_WORLD_HPP
