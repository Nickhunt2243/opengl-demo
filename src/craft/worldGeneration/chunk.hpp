//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_Chunk_HPP
#define OPENGLDEMO_Chunk_HPP

#include <unordered_map>
#include <functional>
#include <thread>
#include <bitset>

#include "block.hpp"
#include "../misc/coordinate.hpp"
#include "../misc/globals.hpp"
#include "../misc/textures.hpp"
#include "../../helpers/threadPool.hpp"
#include "../../setup/program.hpp"

namespace Craft
{
    class Chunk
    {
    public:
        Chunk(
            Engine::Program* blockProgram,
            int x, int z,
            std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords,
            std::mutex* coordsMutex, GLuint VBO
        );
        ~Chunk() = default;
        /// Initialize a Chunk found at the x, z coordinates.
        void initChunk(NeighborInfo* visibility);
        /// A helper function for initializing all neighbor information.
        void initBufferData(Textures* textures, int chunkIdx);
        /// A bitset for every block in the chunk. Set to true if a block occupies that space, otherwise false.
        std::unordered_map<Coordinate<int>, Block> blockCoords{0};
        /**
         * Create a block at the given position.
         *
         * @param blockPos:   The position at which to create a block.
         * @param visibility: The neighbor information for the given chunk.
         */
        void createBlock(Coordinate<int> blockPos, NeighborInfo* visibility);
        /**
         * Delete a block at the given position.
         *
         * @param blockPos:   The position at which to delete the block.
         * @param visibility: The neighbor information for the given chunk.
         */
        void deleteBlock(Coordinate<int> blockPos, NeighborInfo* visibility);
        /// The size of the VBO (numBlocks * vertices per block)
        int vboSize{0};
    private:
        /// The Vertex Buffer Object of the OpenGL program.
        GLuint VBO{0};
        /// A mutex for accessing the coords set.
        std::mutex* coordsMutex;
        /// A mutex for creating/accessing blocks
        std::mutex blocksMutex{};
        /// The array of all blocks within this chunk.
        std::unordered_map<Coordinate<int>, Block> blocksMap{};
        /// The 2D coordinate (x and z) of the chunk.
        Coordinate2D<int> chunkPos;
        /// The unsigned integer of the current program.
        Engine::Program* blockProgram;
        /// A pointer to the mapping of chunk coordinate to block coord map.
        std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords;
    };
}

#endif //OPENGLDEMO_Chunk_HPP
