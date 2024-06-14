//
// Created by admin on 5/26/2024.
//
#include <iostream>

#include <string>
#include <unordered_set>
#include <sstream>
#include <mutex>
#include <random>

#include "chunk.hpp"
#define Y_STEP_VALUE 8
#define VERT_STEP_VALUE 16 * 16
#define CHUNK_OFFSET 0.5f
namespace Craft
{
    Textures* Chunk::textures = nullptr;

    Chunk::Chunk(
            GLuint programId,
            float x, float z,
            std::unordered_map<Coordinate2D, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords,
            std::mutex* coordsMutex
    )
        : programId{programId}
        , coords{coords}
        , chunkPos(x, z)
        , coordsMutex{coordsMutex}
    {};
    Chunk::~Chunk()
    {
        delete[] vertexBufferData;
        delete[] elementBuffer;
        for (auto block: blocks) {
            delete block;
        }
        {
            std::lock_guard<std::mutex> lock(*coordsMutex);
            coords->erase(chunkPos);
        }
        if (EBO != 0 && VBO != 0 && VAO != 0)
        {
            glDeleteBuffers(1, &(EBO));
            glDeleteBuffers(1, &(VBO));
            glDeleteVertexArrays(1, &(VAO));
        }
    }

    void Chunk::initChunk(int chunkOriginX, int chunkOriginZ) {

        BlockType blockType = BlockType::STONE;
        std::random_device rd; // Seed the random number generator with a non-deterministic value
        std::mt19937 gen(rd()); // Mersenne Twister generator
        // Create a distribution for integers in the range [min, max]
        std::uniform_int_distribution<> dis(-1, 0);
        // Generate a random number
        int random_number;
        random_number = dis(gen);
        if (random_number == -1) {
            blockType = BlockType::STONE;
        } else if (random_number == 0) {
            blockType = BlockType::DIRT;
        } else {
            blockType = BlockType::GRASS;
        }
        int bitsetIdx;
        random_number = dis(gen);
        if (random_number == -1) {
            blockType = BlockType::STONE;
        } else if (random_number == 0) {
            blockType = BlockType::DIRT;
        } else {
            blockType = BlockType::GRASS;
        }
        for (int xIdx=0; xIdx<CHUNK_WIDTH; xIdx++)
        {
            for (int zIdx=0; zIdx<CHUNK_WIDTH; zIdx++)
            {
                for (int yIdx=0; yIdx<CHUNK_HEIGHT; yIdx++)
                {

                    bitsetIdx = (yIdx * CHUNK_WIDTH * CHUNK_WIDTH) + (zIdx * CHUNK_WIDTH) + xIdx;
                    Coordinate coord{(float) xIdx, (float) yIdx, (float) zIdx};
                    blockTexture texture = textures->getTexture(blockType);
                    auto newBlock = new Block(coord, texture, blockType);
                    blocks.push_back(newBlock);
                    blockCoords[bitsetIdx] = true;
                }
            }
        }
        {
            std::lock_guard<std::mutex> lock(*coordsMutex);
            coords->insert({chunkPos, &blockCoords});
        }
    }
    void Chunk::updateNeighborInfo()
    {
        int bitsetIdx;
        int blockIdx = 0;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* leftChunkBitMap = nullptr;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* rightChunkBitMap = nullptr;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* frontChunkBitMap = nullptr;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* backChunkBitMap = nullptr;
        Coordinate2D leftChunkCoord = Coordinate2D{chunkPos.x - 1.0f, chunkPos.z};
        Coordinate2D rightChunkCoord = Coordinate2D{chunkPos.x + 1.0f, chunkPos.z};
        Coordinate2D frontChunkCoord = Coordinate2D{chunkPos.x, chunkPos.z + 1.0f};
        Coordinate2D backChunkCoord = Coordinate2D{chunkPos.x, chunkPos.z - 1.0f};
        if (coords->find(leftChunkCoord) != coords->end()) {
            leftChunkBitMap = (*coords)[leftChunkCoord];
        }
        if (coords->find(rightChunkCoord) != coords->end()) {
            rightChunkBitMap = (*coords)[rightChunkCoord];
        }
        if (coords->find(frontChunkCoord) != coords->end()) {
            frontChunkBitMap = (*coords)[frontChunkCoord];
        }
        if (coords->find(backChunkCoord) != coords->end()) {
            backChunkBitMap = (*coords)[backChunkCoord];
        }


        for (int x=0; x<CHUNK_WIDTH; x++)
        {
            for (int z=0; z<CHUNK_WIDTH; z++)
            {
                for (int y=0; y<CHUNK_HEIGHT; y++)
                {
                    // 0 * 128
                    bitsetIdx = (y * CHUNK_WIDTH * CHUNK_WIDTH) + (z * CHUNK_WIDTH) + x;
                    // If block is air, skip
                    if (!blockCoords[bitsetIdx]) {
                        continue;
                    }
                    // Block is front
                    if (z == 15 && frontChunkBitMap != nullptr)
                    {
                        updateFrontEdgeNeighbors(*frontChunkBitMap, x, y, z, blocks[blockIdx]);
                    }
                    else if (z == 15)
                    {
                        blocks[blockIdx]->neighborInfo.front = 1;
                    }
                    // Block is right side
                    if (x == 15 && rightChunkBitMap != nullptr)
                    {
                        updateRightEdgeNeighbors(*rightChunkBitMap, x, y, z, blocks[blockIdx]);
                    }
                    else if (x == 15)
                    {
                        blocks[blockIdx]->neighborInfo.right = 1;
                    }
                    // Block is back
                    if (z == 0 && backChunkBitMap != nullptr)
                    {
                        updateBackEdgeNeighbors(*backChunkBitMap, x, y, z, blocks[blockIdx]);
                    }
                    else if (z == 0)
                    {
                        blocks[blockIdx]->neighborInfo.back = 1;
                    }
                    // Block is left side
                    if (x == 0 && leftChunkBitMap != nullptr)
                    {
                        updateLeftEdgeNeighbors(*leftChunkBitMap, x, y, z, blocks[blockIdx]);
                    }
                    else if (x == 0)
                    {
                        blocks[blockIdx]->neighborInfo.left = 1;
                    }
                    updateNeighbors(blockCoords, bitsetIdx, x, y, z, blocks[blockIdx]);
                    blockIdx++;
                }
            }
        }
    }
    void Chunk::updateChunkFront(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* frontChunkBitMap
        )
    {
        int z = 15;
        int blockIdx = 0;
        bool rerunElements = true;
        for (int x = 0; x < CHUNK_WIDTH; x++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                bool result = updateFrontEdgeNeighbors(*frontChunkBitMap, x, y, z, blocks[blockIdx]);
                rerunElements |= result;
                blockIdx++;
            }
            blockIdx += CHUNK_WIDTH;
        }
        needToInitElements = rerunElements;
    }
    void Chunk::updateChunkRight(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* rightChunkBitMap
        )
    {
        int x = 15;
        int blockIdx = 15 * CHUNK_HEIGHT * CHUNK_WIDTH;
        bool rerunElements = true;
        for (int z = 0; z < CHUNK_WIDTH; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                bool result = updateRightEdgeNeighbors(*rightChunkBitMap, x, y, z, blocks[blockIdx]);
                rerunElements |= result;
                blockIdx++;
            }
        }
        needToInitElements = rerunElements;
    }
    void Chunk::updateChunkBack(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* backChunkBitMap
        )
    {
        int z = 0;
        int blockIdx = 0;
        bool rerunElements = true;
        for (int x = 0; x < CHUNK_WIDTH; x++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                bool result = updateBackEdgeNeighbors(*backChunkBitMap, x, y, z, blocks[blockIdx]);
                rerunElements |= result;
                blockIdx++;
            }
            blockIdx = x * CHUNK_HEIGHT * CHUNK_WIDTH;
        }
        needToInitElements = rerunElements;
    }
    void Chunk::updateChunkLeft(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* leftChunkBitMap
        )
    {
        int x = 0;
        int blockIdx = 0;
        bool rerunElements = true;
        for (int z = 0; z < CHUNK_WIDTH; z++)
        {
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                bool result = updateBackEdgeNeighbors(*leftChunkBitMap, x, y, z, blocks[blockIdx]);
                rerunElements |= result;
                blockIdx++;
            }
        }
        needToInitElements = rerunElements;
    }
    void Chunk::initElementBuffer()
    {
        elementCount = 0;
        for (auto & block: blocks)
        {
            elementCount += getElementSize(block);
        }
        delete elementBuffer;
        elementBuffer = new unsigned int[elementCount];
        int eIdx = 0,
            currBlocksVerticesIdx = 0;
        int count = 0;
        Block* currBlock;
        for ( int i=0; i<blocks.size(); i++ )
        {
            currBlock = blocks[i];
            currBlocksVerticesIdx += 6 * 4;
            eIdx += getElementSize(currBlock);
            count++;
            if (count == blocks.size() / VERT_STEP_VALUE)
            {
                fillElementBuffer(i, eIdx, currBlocksVerticesIdx);
                count = 0;
            }
        }
        needToInitElements = true;
    }
    void Chunk::initBufferData()
    {
        vboSize = (int) blocks.size() * getVerticesCount();
        vertexBufferData = new int[vboSize];
        int vIdx = 0;
        int count = 0;
        for ( int i=0; i<blocks.size(); i++ )
        {
            // Testing with one block.
            vIdx += getVerticesCount();
            count++;
            if (count == blocks.size() / VERT_STEP_VALUE)
            {
                fillVertexBuffer(i, vIdx);
                count = 0;
            }
        }
        isReadyToInitVAO = true;
    }
    void Chunk::fillElementBuffer(
            int endIdx, int startingEIdx, int currBlocksVerticesIdx
        )
    {
        int startIdx = endIdx - ((int) blocks.size() / VERT_STEP_VALUE) + 1;
        int eIdx = startingEIdx;
        Block* currBlock;

        for (int i=endIdx; i>=startIdx; i--)
        {
            currBlock = blocks[i];
            currBlocksVerticesIdx -= 6 * 4;
            eIdx -= getElementSize(currBlock);
            fillElementBufferData(currBlock, elementBuffer, eIdx, currBlocksVerticesIdx);
        }
    }
    void Chunk::fillVertexBuffer(
            int endIdx, int startingVIdx
        )
    {
        int startIdx = endIdx - ((int) blocks.size() / VERT_STEP_VALUE) + 1;
        int vIdx = startingVIdx;
        Block* currBlock;

        for (int i=endIdx; i>=startIdx; i--)
        {
            currBlock = blocks[i];
            vIdx -= getVerticesCount();
            fillVerticesBufferData(currBlock, vertexBufferData, vIdx);
        }
    }
    void Chunk::initEBO()
    {
        // Initialize the element buffer.
        glBindVertexArray(VAO);
        // Bind and initialize EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        auto indexSize = (GLsizeiptr) (elementCount * sizeof(GLuint));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, elementBuffer, GL_DYNAMIC_DRAW);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        needToInitElements = true;
        // We need VAO (and VBO by default) to be initialized prior to calling initEBO so after this we can draw.
        canDrawChunk = true;
    }
    void Chunk::initVAO()
    {
        // Init VAO and VBO
        if (VAO == 0)
            glGenVertexArrays(1, &VAO);
        if (VBO == 0)
            glGenBuffers(1, &VBO);
        if (EBO == 0)
            glGenBuffers(1, &EBO);

        // Bind VAO
        glBindVertexArray(VAO);
        // Bind and initialize the VBO
        // Per vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        auto size = (GLsizeiptr) (vboSize * sizeof(int));
        glBufferData(GL_ARRAY_BUFFER, size, vertexBufferData, GL_STATIC_DRAW);
        GLsizei stride =  (2) * sizeof(int);

        glVertexAttribIPointer(0, 1, GL_INT, stride, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(1, 1, GL_INT, stride, (void*)(1 * sizeof(int)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void Chunk::drawChunk()
    {
        if (VAO == 0 && isReadyToInitVAO)
        {
            initVAO();
        }
        if (VAO != 0 && needToInitElements) {
            initEBO();
        }
        if (canDrawChunk) {
            glBindVertexArray(VAO);
            setVec2(programId, "u_ChunkPos", (chunkPos - CHUNK_OFFSET) * 16.0f);
            glDrawElements(GL_TRIANGLES, (GLint) elementCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
}