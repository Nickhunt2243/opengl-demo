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
        Chunk(std::unordered_set<size_t>* coords);
        Chunk();
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
        void findNeighbors();
        /// A static textures object.
        static Textures* textures;
        /// A function for initializing the buffer's of the chunk.
        void initBuffers();
    private:
        /// The Vertex Array Object of the OpenGL program.
        GLuint VAO{0},
        /// The Vertex Buffer Object of the OpenGL program.
               VBO{0},
        /// The Element Buffer Object of the OpenGL program;
               EBO{0};
        /// The pool instance of the chunk.
        ThreadPool pool{std::thread::hardware_concurrency()};
        /// The array of futures to be ran through the thread pool.
        std::vector<std::future<void>> futures{};
        /// The set of all Coords.
        std::unordered_set<size_t>* coords;
        /// The array of all blocks within this chunk.
        Block** blocks = nullptr;
        /// The vertex buffer array.
        float* vertexBufferData{};
        /// The element buffer array.
        unsigned int* elementBuffer{};
        /// The number of blocks to draw within this chunk.
        int numBlocks{0};
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
        void fillBuffers(Block* currBlock, int vIdx, int eIdx, int currIdx);
        /**
         * A function to update neighbor info within the block.
         *
         * Figure out which sides of the block have a block next to them.
         *
         * @param idx: The index of block in the blocks array.
         */
        void updateNeighborInfo(int idx);
        /**
         * A helper function for initializing a layer of the chunk. Mostly used for threading.
         *
         * TODO: Update this function to go z and y axis rather than x and z. This will help with varying height.
         *
         * @param xStart:    The starting x index.
         * @param xEnd:      The ending x index.
         * @param zStart:    The starting z index.
         * @param zEnd:      The ending z index.
         * @param y:         The y index.
         * @param idx:       The starting index of the blocks to initialize.
         * @param blockType: The current type of the block.
         *                      - (will have to remove when updating the function to revolve around y value)
         */
        void initLayer(
                int xStart, int xEnd,
                int zStart, int zEnd,
                float y, int idx,
                BlockType blockType
        );

    };
}

#endif //OPENGLDEMO_Chunk_HPP
