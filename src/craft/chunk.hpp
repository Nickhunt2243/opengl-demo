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
            std::mutex* mutex
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
        void drawChunk() const;
        /// A helper function for initializing all neighbor information.
        void initBufferData();
        /// A static textures object.
        static Textures* textures;
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
        std::mutex* mutex;
        /// The array of all blocks within this chunk.
//        Block** blocks = nullptr;
        std::vector<Block*> blocks{};
        /// The vertex buffer array.
        float* vertexBufferData{};
        /// The element buffer array.
        unsigned int* elementBuffer{};
        /// The size of the VBO (numBlocks * vertices per block)
        int vboSize{0};
        /// The number of elements to draw.
        int elementCount{0};
        /// Initialize the Vertex Array Object.
        void initVAO();
        /**
         * A function to fill all of the buffers being used to render a chunk.
         *
         * @param currBlock:  The current block to be initialized.
         * @param vIdx:       The current index of the vertex buffer.
         * @param eIdx:       The current index of the element buffer.
         * @param currIdx:    The current index of the vertex to be drawn (the element.)
         */
        void fillBuffers(int idx, int startingVIdx, int startingEIdx, int currBlocksVerticesIdx);
        /**
         * A function to update neighbor info within the block.
         *
         * Figure out which sides of the block have a block next to them.
         *
         * @param idx: The index of block in the blocks array.
         */
        void updateNeighborInfo(int startIdx, int endIdx);

    };
}

#endif //OPENGLDEMO_Chunk_HPP
