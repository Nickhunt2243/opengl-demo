//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_Chunk_HPP
#define OPENGLDEMO_Chunk_HPP

#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif
#ifndef CHUNK_HEIGHT
#define CHUNK_HEIGHT 128
#endif
#include <unordered_map>
#include <functional>
#include <thread>
#include <bitset>

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
            GLuint programId,
            float x, float z,
            std::unordered_map<Coordinate2D, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords,
            std::mutex* coordsMutex
        );
        ~Chunk();
        /// Initialize a Chunk found at the x, z coordinates.
        void initChunk();
        /// A function to draw the chunk using OpenGL.
        void drawChunk();
        /// A helper function for initializing all neighbor information.
        void initBufferData();
        /// A static textures object.
        static Textures* textures;
        /// Initialize the Element Buffer array.
        void initElementBuffer();
        /**
         * A function to update neighbor info within the block.
         *
         * Figure out which sides of the block have a block next to them.
         *
         * @param idx: The index of block in the blocks array.
         */
        void updateNeighborInfo();
        /**
         * Determine whether to draw the entire front edge of a chunk based on whether the
         * chunk next to it has blocks.
         *
         * @param frontChunkBitMap: The Chunk that is +1 z away from the current.
         */
        void updateChunkFront(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* frontChunkBitMap
        );
        /**
         * Determine whether to draw the entire right edge of a chunk based on whether the
         * chunk next to it has blocks.
         *
         * @param rightChunkBitMap: The Chunk that is +1 x away from the current.
         */
        void updateChunkRight(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* rightChunkBitMap
        );
        /**
         * Determine whether to draw the entire back edge of a chunk based on whether the
         * chunk next to it has blocks.
         *
         * @param backChunkBitMap: The Chunk that is -1 z away from the current.
         */
        void updateChunkBack(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* backChunkBitMap
        );
        /**
         * Determine whether to draw the entire left edge of a chunk based on whether the
         * chunk next to it has blocks.
         *
         * @param leftChunkBitMap: The Chunk that is -1 x away from the current.
         */
        void updateChunkLeft(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* leftChunkBitMap
        );
        /// A bitset for every block in the chunk. Set to true if a block occupies that space, otherwise false.
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> blockCoords{0};
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
        /// The array of all blocks within this chunk.
        std::vector<Block*> blocks{};
        /// The vertex buffer array.
        int* vertexBufferData{nullptr};
        /// The element buffer array.
        unsigned int* elementBuffer{nullptr};
        /// The size of the VBO (numBlocks * vertices per block)
        int vboSize{0};
        /// The number of elements to draw.
        int elementCount{0};
        /// The 2D coordinate (x and z) of the chunk.
        Coordinate2D chunkPos;
        /// The unsigned integer of the current program.
        GLuint programId;
        /// A pointer to the mapping of chunk coordinate to block coord bitset.
        std::unordered_map<Coordinate2D, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords;
        /// A Boolean of whether we need to initialize the EBO
        bool needToInitElements{false};
        /// Initialize the Element Buffer Object.
        void initEBO();
        /// Initialize the Vertex Array Object.
        void initVAO();
    };
}

#endif //OPENGLDEMO_Chunk_HPP
