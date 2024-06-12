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
#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 128
#define Y_STEP_VALUE 8
#define VERT_STEP_VALUE 16 * 16
namespace Craft
{
    Textures* Chunk::textures = nullptr;

    Chunk::Chunk(
            std::unordered_set<size_t>* coords,
            ThreadPool* pool,
            std::vector<std::future<void>>* futures,
            std::mutex* coordsMutex
    )
        : coords{coords}
        , pool{pool}
        , futures{futures}
        , coordsMutex{coordsMutex}
    {};
    Chunk::~Chunk()
    {
        delete[] vertexBufferData;
        delete[] elementBuffer;
        for (auto block: blocks) {
            coords->erase(block->coord->add(0, 0, 0));
            delete block;
        }
        if (VAO != 0)
        {
            glDeleteBuffers(1, &(EBO));
            glDeleteBuffers(1, &(VBO));
            glDeleteVertexArrays(1, &(VAO));
        }
    }

    void Chunk::initChunk(int chunkOriginX, int chunkOriginZ) {

        int xStart = chunkOriginX - (CHUNK_WIDTH / 2),
            xEnd = chunkOriginX + (CHUNK_WIDTH / 2),
            zStart = chunkOriginZ - (CHUNK_WIDTH / 2),
            zEnd = chunkOriginZ + (CHUNK_WIDTH / 2);

        BlockType blockType = BlockType::STONE;
        std::random_device rd; // Seed the random number generator with a non-deterministic value
        std::mt19937 gen(rd()); // Mersenne Twister generator
        // Create a distribution for integers in the range [min, max]
        std::uniform_int_distribution<> dis(-1, 1);
        // Generate a random number
        int random_number;
        random_number = dis(gen);
        if (random_number == -1) {
            blockType = BlockType::STONE;
        } else if (random_number == 0) {
            blockType = BlockType::GRASS;
        } else {
            blockType = BlockType::DIRT;
        }

//        for (int xIdx=0; xIdx<CHUNK_WIDTH; xIdx++)
//        {
//            for (int zIdx=0; zIdx<CHUNK_WIDTH; zIdx++)
//            {
//                for (int yIdx=0; yIdx<CHUNK_HEIGHT; yIdx++)
//                {
//                    // Create blocks passing the
//                }
//            }
//        }

        for (int xCoord=xStart; xCoord<xEnd; xCoord++ )
        {
            for (int zCoord=zStart; zCoord<zEnd;zCoord++)
            {
                for (int yCoord=random_number; yCoord>-CHUNK_HEIGHT; yCoord--)
                {
                    auto coord = new Coordinate((float) xCoord, (float) yCoord, (float) zCoord);
                    auto newBlock = new Block();
                    blockTexture texture = textures->getTexture(blockType);
                    newBlock->coord = coord;
                    newBlock->type = blockType;
                    newBlock->textures = texture;
                    blocks.push_back(newBlock);
                    size_t hashedCoord = coord->add(0.0f, 0.0f, 0.0f);
                    {
                        std::lock_guard<std::mutex> lock(*coordsMutex);
                        coords->insert(hashedCoord);
                    }
                }
            }
        }
    }
    void Chunk::updateNeighborInfo(int startIdx, int endIdx)
    {
        for (int i=startIdx; i<endIdx; i++)
        {
            updateNeighbors(coords, coordsMutex, blocks[i]);
        }
    }
    void Chunk::initElementBuffer()
    {
        elementCount = 0;
        int startIdx = 0, endIdx = 0;
        for ( int i=0; i<blocks.size() / VERT_STEP_VALUE;i++)
        {
            endIdx += VERT_STEP_VALUE;
            if (endIdx <= blocks.size())
            {
                updateNeighborInfo(startIdx, endIdx);
            }
            startIdx = endIdx;
        }
        for (auto & block : blocks)
        {
            elementCount += getElementSize(block);
        }

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

        isReadyToInitVAO = true;
    }
    void Chunk::initBufferData()
    {
        vboSize = (int) blocks.size() * getVerticesCount();
//        elementCount = 0;
//        int startIdx = 0, endIdx = 0;
//        for ( int i=0; i<blocks.size() / VERT_STEP_VALUE;i++)
//        {
//            endIdx += VERT_STEP_VALUE;
//            if (endIdx <= blocks.size())
//            {
//                updateNeighborInfo(startIdx, endIdx);
//            }
//            startIdx = endIdx;
//        }
//        for (auto & block : blocks)
//        {
//            elementCount += getElementSize(block);
//        }
//        Block* currBlock;
        vertexBufferData = new float[vboSize];
//        elementBuffer = new unsigned int[elementCount];
        int vIdx = 0;
//            eIdx = 0,
//            currBlocksVerticesIdx = 0;
        int count = 0;
        for ( int i=0; i<blocks.size(); i++ )
        {
//            currBlock = blocks[i];
//            currBlocksVerticesIdx += 6 * 4;
//            eIdx += getElementSize(currBlock);
            vIdx += getVerticesCount();
            count++;
            if (count == blocks.size() / VERT_STEP_VALUE)
            {
//                fillElementBuffer(i, eIdx, currBlocksVerticesIdx);
                fillVertexBuffer(i, vIdx);
                count = 0;
            }
        }
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
        auto size = (GLsizeiptr) (vboSize * sizeof(float)) ;
        glBufferData(GL_ARRAY_BUFFER, size, vertexBufferData, GL_STATIC_DRAW);
        GLsizei stride =  (10) * sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);

        // Bind and initialize EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        auto indexSize = (GLsizeiptr) (elementCount * sizeof(GLuint));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, elementBuffer, GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        canDrawChunk = true;
    }
    void Chunk::drawChunk()
    {
        if (VAO == 0 && isReadyToInitVAO)
        {
            initVAO();
        }
        if (canDrawChunk) {
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, (GLint) elementCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
}