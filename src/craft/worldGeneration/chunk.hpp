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
            std::mutex* coordsMutex
        );
        ~Chunk();
        /// Initialize a Chunk found at the x, z coordinates.
        void initChunk(RowNeighborInfo* visibility);
        /**
         * A function to draw the chunk using OpenGL.
         *
         * For information on how I find the default light level view:
         * https://www.desmos.com/calculator/yse4g8xec5
         *
         * @param gameTime: A struct containing the in-game time.
         */
        /// A function to draw the chunk using OpenGL.
        void drawChunk(Time gameTime);
        /// A helper function for initializing all neighbor information.
        void initBufferData();
        /// A static textures object.
        static Textures* textures;
        /// Initialize the Element Buffer array.
        void initElementBuffer(RowNeighborInfo* visibility);
        /// A bitset for every block in the chunk. Set to true if a block occupies that space, otherwise false.
        std::unordered_map<Coordinate<int>, Block> blockCoords{0};
        /**
         * Create a block at the given position.
         *
         * @param blockPos:   The position at which to create a block.
         * @param visibility: The neighbor information for the given chunk.
         */
        void createBlock(Coordinate<int> blockPos, RowNeighborInfo* visibility);
        /**
         * Delete a block at the given position.
         *
         * @param blockPos:   The position at which to delete the block.
         * @param visibility: The neighbor information for the given chunk.
         */
        void deleteBlock(Coordinate<int> blockPos, RowNeighborInfo* visibility);
    private:
        /// The Vertex Array Object of the OpenGL program.
        GLuint VAO{0},
        /// The Vertex Buffer Object of the OpenGL program.
               VBO{0},
        /// The Element Buffer Object of the OpenGL program;
               EBO{0};
        /// A Boolean of whether we are ready to initialize the VAO
        bool isReadyToInitVAO{false};
        /// A Boolean to know if we are ready to start drawing.
        bool canDrawChunk{false};
        /// A mutex for accessing the coords set.
        std::mutex* coordsMutex;
        /// A mutex for creating/accessing blocks
        std::mutex blocksMutex{};
        /// The array of all blocks within this chunk.
        std::unordered_map<Coordinate<int>, Block> blocksMap{};
        /// The vertex buffer array.
        int* vertexBufferData{nullptr};
        /// The element buffer array.
        uint32_t* elementBuffer{nullptr};
        /// The size of the VBO (numBlocks * vertices per block)
        int vboSize{0};
        /// The number of elements to draw.
        int elementCount{0};
        /// The 2D coordinate (x and z) of the chunk.
        Coordinate2D<int> chunkPos;
        /// The unsigned integer of the current program.
        Engine::Program* blockProgram;
        /// A pointer to the mapping of chunk coordinate to block coord map.
        std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords;
        /// A Boolean of whether we need to initialize the EBO
        bool needToInitElements{false};
        void initEBO();
        /// Initialize the Vertex Array Object.
        void initVAO();
    };
}

#endif //OPENGLDEMO_Chunk_HPP
