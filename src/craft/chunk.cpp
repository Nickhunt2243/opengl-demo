#include <string>
#include <unordered_set>
#include <sstream>
#include <mutex>
#include <random>

#include "chunk.hpp"
#include "../helpers/noise.hpp"
#define CHUNK_OFFSET 0.5f
#define CHUNK_BASE_HEIGHT 100;
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

    void Chunk::initChunk() {
        BlockType blockType = BlockType::GRASS;

        int bitsetIdx;
        std::random_device rd; // Seed the random number generator with a non-deterministic value
        std::mt19937 gen(44); // Mersenne Twister generator
        // Create a distribution for integers in the range [min, max]
        std::uniform_int_distribution<> dis(1, 100);
        // Generate a random number
        float random_amplitude = (float) dis(gen) / 100.0f;
        float random_frequency = (float) dis(gen) / 100.0f;
        Craft::Noise noiseGenerator(44);

        for (int xIdx=0; xIdx<CHUNK_WIDTH; xIdx++)
        {
            for (int zIdx=0; zIdx<CHUNK_WIDTH; zIdx++)
            {
                float yHeight = noiseGenerator.fractalNoise(
                    ((chunkPos.x * 16.0f) + (float) xIdx) / 7,
                    100.0f / 7,
                    ((chunkPos.z * 16.0f) + (float) zIdx) / 7,
                    10,
                    0.25f,
                    random_amplitude,
                    random_frequency
                ) * 7;
                float yHeightFloat = yHeight + CHUNK_BASE_HEIGHT;
                int yHeightFinal = (int) round(yHeightFloat);
                for (int yIdx=0; yIdx<yHeightFinal; yIdx++)
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
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* leftChunkBitMap = nullptr;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* rightChunkBitMap = nullptr;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* frontChunkBitMap = nullptr;
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* backChunkBitMap = nullptr;
        Coordinate2D frontChunkCoord = Coordinate2D{chunkPos.x, chunkPos.z + 1.0f};
        Coordinate2D rightChunkCoord = Coordinate2D{chunkPos.x + 1.0f, chunkPos.z};
        Coordinate2D backChunkCoord = Coordinate2D{chunkPos.x, chunkPos.z - 1.0f};
        Coordinate2D leftChunkCoord = Coordinate2D{chunkPos.x - 1.0f, chunkPos.z};
        if (coords->find(leftChunkCoord) != coords->end())
        {
            leftChunkBitMap = (*coords)[leftChunkCoord];
        }
        if (coords->find(rightChunkCoord) != coords->end())
        {
            rightChunkBitMap = (*coords)[rightChunkCoord];
        }
        if (coords->find(frontChunkCoord) != coords->end())
        {
            frontChunkBitMap = (*coords)[frontChunkCoord];
        }
        if (coords->find(backChunkCoord) != coords->end())
        {
            backChunkBitMap = (*coords)[backChunkCoord];
        }
        int x, y, z;
        for (auto block: blocks)
        {
            x = (int) block->chunkRelativeCoord.x;
            y = (int) block->chunkRelativeCoord.y;
            z = (int) block->chunkRelativeCoord.z;
            // Block is front
            if (z == 15 && frontChunkBitMap != nullptr)
            {
                updateFrontEdgeNeighbors(*frontChunkBitMap, x, y, z, block);
            }
            else if (z == 15)
            {
                block->neighborInfo.front = 1;
            }
            // Block is right side
            if (x == 15 && rightChunkBitMap != nullptr)
            {
                updateRightEdgeNeighbors(*rightChunkBitMap, x, y, z, block);
            }
            else if (x == 15)
            {
                block->neighborInfo.right = 1;
            }
            // Block is back
            if (z == 0 && backChunkBitMap != nullptr)
            {
                updateBackEdgeNeighbors(*backChunkBitMap, x, y, z, block);
            }
            else if (z == 0)
            {
                block->neighborInfo.back = 1;
            }
            // Block is left side
            if (x == 0 && leftChunkBitMap != nullptr)
            {
                updateLeftEdgeNeighbors(*leftChunkBitMap, x, y, z, block);
            }
            else if (x == 0)
            {
                block->neighborInfo.left = 1;
            }
            updateNeighbors(blockCoords, x, y, z, block);
        }

        needToInitElements = true;
    }
    void Chunk::updateChunkFront(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* frontChunkBitMap
        )
    {
        int z = 15;
        bool rerunElements = true;
        int x, y;
        for (auto block: blocks) {
            if (block->chunkRelativeCoord.z != 15) {
                continue;
            }
            x = (int) block->chunkRelativeCoord.x;
            y = (int) block->chunkRelativeCoord.y;
            bool result = updateFrontEdgeNeighbors(*frontChunkBitMap, x, y, z, block);
            rerunElements |= result;
        }
        needToInitElements = rerunElements;
    }
    void Chunk::updateChunkRight(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* rightChunkBitMap
        )
    {
        int xToCheck = 15;
        bool rerunElements = true;
        int z, y;
        for (auto block: blocks) {
            if (block->chunkRelativeCoord.x != (float) xToCheck) {
                continue;
            }
            z = (int) block->chunkRelativeCoord.z;
            y = (int) block->chunkRelativeCoord.y;
            bool result = updateRightEdgeNeighbors(*rightChunkBitMap, xToCheck, y, z, block);
            rerunElements |= result;
        }
        needToInitElements = rerunElements;
    }
    void Chunk::updateChunkBack(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* backChunkBitMap
        )
    {
        int z = 0;
        bool rerunElements = true;
        int x, y;
        for (auto block: blocks) {
            if (block->chunkRelativeCoord.z != 0) {
                continue;
            }
            x = (int) block->chunkRelativeCoord.x;
            y = (int) block->chunkRelativeCoord.y;

            bool result = updateBackEdgeNeighbors(*backChunkBitMap, x, y, z, block);
            rerunElements |= result;
        }
        needToInitElements = rerunElements;
    }
    void Chunk::updateChunkLeft(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>* leftChunkBitMap
        )
    {
        int x = 0;
        bool rerunElements = true;
        int z, y;
        for (auto block: blocks) {
            if (block->chunkRelativeCoord.x != 0) {
                continue;
            }
            z = (int) block->chunkRelativeCoord.z;
            y = (int) block->chunkRelativeCoord.y;
            bool result = updateLeftEdgeNeighbors(*leftChunkBitMap, x, y, z, block);
            rerunElements |= result;
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
        delete[] elementBuffer;
        elementBuffer = new unsigned int[elementCount];
        int eIdx = 0,
            currBlocksVerticesIdx = 0;
        for (auto block: blocks) {
            fillElementBufferData(block, elementBuffer, eIdx, currBlocksVerticesIdx);
            currBlocksVerticesIdx += 6 * 4;
            eIdx += getElementSize(block);
        }
    }
    void Chunk::initBufferData()
    {
        vboSize = (int) blocks.size() * getVerticesCount();
        delete[] vertexBufferData;
        vertexBufferData = new int[vboSize];
        int vIdx = 0;
        for (auto block: blocks)
        {
            // Testing with one block.
            fillVerticesBufferData(block, vertexBufferData, vIdx);
            vIdx += getVerticesCount();
        }
        isReadyToInitVAO = true;
    }
    void Chunk::initEBO()
    {
        initElementBuffer();
        // Initialize the element buffer.
        glBindVertexArray(VAO);
        // Bind and initialize EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        auto indexSize = (GLsizeiptr) (elementCount * sizeof(GLuint));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, elementBuffer, GL_STATIC_DRAW);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // We need VAO (and VBO by default) to be initialized prior to calling initEBO so after this we can draw.
        needToInitElements = false;
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