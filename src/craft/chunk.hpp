//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_Chunk_HPP
#define OPENGLDEMO_Chunk_HPP

#include <unordered_map>
#include <functional>
#include <thread>

#include "../helpers/threadPool.hpp"
#include "block.hpp"
#include "textures.hpp"
#include "coordinate.hpp"

namespace Craft
{
    class Chunk
    {
    public:
        Chunk(
            std::unordered_set<size_t>* coords,
            ThreadPool* pool,
            std::vector<std::future<void>>* futures,
            std::mutex* coordsMutex
        );
        ~Chunk();
        /**
         * Initialize a Chunk found at the x, z coordinates.
         *
         * @param x: The x coordinate of the center of the chunk.
         * @param z: The z coordinate of the center of the chunk.
         */
        void initChunk(int x, int z);
        /// A function to draw the chunk using OpenGL.
        void drawChunk();
        /// A helper function for initializing all neighbor information.
        void initBufferData();
        /// A static textures object.
        static Textures* textures;
        /// Initialize the Element Buffer array.
        void initElementBuffer();
    private:
        /// The Vertex Array Object of the OpenGL program.
        GLuint VAO{0},
        /// The Vertex Buffer Object of the OpenGL program.
               VBO{0},
        /// The Element Buffer Object of the OpenGL program;
               EBO{0};
        /// The pool instance of the chunk.
        ThreadPool* pool;
        /// The array of futures to be ran through the thread pool.
        std::vector<std::future<void>>* futures;
        /// The set of all Coords.
        std::unordered_set<size_t>* coords;
        /// A Boolean of whether we are ready to initialize the VAO
        bool isReadyToInitVAO{false};
        /// A Boolean to know if we are ready to start drawing.
        bool canDrawChunk{false};
        /// A mutex for accessing the coords set.
        std::mutex* coordsMutex;
        /// The array of all blocks within this chunk.
        std::vector<Block*> blocks{};
        /// The vertex buffer array.
        float* vertexBufferData{nullptr};
        /// The element buffer array.
        unsigned int* elementBuffer{nullptr};
        /// The size of the VBO (numBlocks * vertices per block)
        int vboSize{0};
        /// The number of elements to draw.
        int elementCount{0};
        /**
         * A function to update neighbor info within the block.
         *
         * Figure out which sides of the block have a block next to them.
         *
         * @param idx: The index of block in the blocks array.
         */
        void updateNeighborInfo(int startIdx, int endIdx);
        /// Initialize the Vertex Array Object.
        void initVAO();
        /**
         * Fill the element buffer with the elements to be drawn.
         *
         * @param endIdx:                The ending idx of blocks.
         * @param startingEIdx:          The starting index of the element buffer.
         * @param currBlocksVerticesIdx: The current number of the vertex buffer to draw.
         */
        void fillElementBuffer(
                int endIdx, int startingEIdx, int currBlocksVerticesIdx
        );
        /**
         * Fill the vertex buffer with the vertex information.
         *
         * @param endIdx:       The ending index of blocks.
         * @param startingVIdx: The starting index of the vertex buffer.
         */
        void fillVertexBuffer(
                int endIdx, int startingVIdx
        );
    };
}

#endif //OPENGLDEMO_Chunk_HPP
