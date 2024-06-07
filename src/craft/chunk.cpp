//
// Created by admin on 5/26/2024.
//
#include <iostream>

#include <string>
#include <unordered_set>
#include <sstream>
#include <mutex>

#include "chunk.hpp"
#include "../helpers/helpers.hpp"
#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 128

namespace Craft
{
    Textures* Chunk::textures = nullptr;

    Chunk::Chunk(std::unordered_set<size_t>* coords)
        : coords{coords}
    {};
    Chunk::Chunk()
            : coords{}
    {};
    Chunk::~Chunk()
    {
        delete[] vertexBufferData;
        if (blocks != nullptr)
        {
            for (int i = 0; i < numBlocks; ++i)
            {
                delete blocks[i];
            }
            delete[] blocks;
        }

        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &(VAO));
            glDeleteBuffers(1, &(VBO));
            glDeleteBuffers(1, &(EBO));
        }
    }
    static std::mutex mutex{};
    void Chunk::initLayer(
            int xStart, int xEnd,
            int zStart, int zEnd,
            float y, int idx,
            BlockType blockType
        )
    {
        for (int x=xStart; x<xEnd; x++)
        {
            for (int z=zStart; z<zEnd; z++)
            {
                auto coord = new Coordinate((float) x, y, (float) z);
                auto newBlock = new Block;
                blockTexture texture = textures->getTexture(blockType);
                newBlock->coord = coord;
                newBlock->type = blockType;
                newBlock->textures = texture;

                std::lock_guard<std::mutex> lock(mutex);
                if (idx >= 0 && idx < numBlocks)
                {
                    blocks[idx] = newBlock;
                    size_t hashedCoord = std::hash<Coordinate*>()(coord);
                    coords->insert(hashedCoord);
                }
                else
                {
                    std::cerr << "Index out of bounds: " << idx << std::endl;
                    delete newBlock; // Avoid memory leak
                }
                idx += 1;
            }
        }
    }

    void Chunk::initChunk(int x, int z)
    {
        BlockType blockType = BlockType::GRASS;

        int xStart = x * CHUNK_SIZE - (CHUNK_SIZE / 2),
            xEnd = x * CHUNK_SIZE + (CHUNK_SIZE / 2),
            zStart = z * CHUNK_SIZE - (CHUNK_SIZE / 2),
            zEnd = z * CHUNK_SIZE + (CHUNK_SIZE / 2),
            yEnd = -CHUNK_HEIGHT;
        numBlocks = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
        blocks = new Block*[numBlocks];
        int idx = 0;
        for (int i = 0; i < numBlocks; ++i) {
            blocks[i] = nullptr;
        }
        futures.clear();
        for (int y=0;y>yEnd;y--)
        {
            futures.emplace_back(
                pool.enqueue(
                    &Chunk::initLayer,
                    this,
                    xStart, xEnd,
                    zStart, zEnd,
                    (float)y, idx,
                    blockType
                )
            );
            idx += CHUNK_SIZE * CHUNK_SIZE;
            if (y < -1)
            {
                blockType = BlockType::STONE;
            }
            else if (y < 1)
            {
                blockType = BlockType::DIRT;
            }
        }
        for (auto& future : futures)
        {
            future.get(); // Wait for all tasks to complete
        }
    }
    void Chunk::fillBuffers(
            Block* currBlock, int vIdx, int eIdx, int currIdx
        )
    {
        fillVerticesBufferData(currBlock, vertexBufferData, vIdx);
        fillElementBufferData(currBlock, elementBuffer, eIdx, currIdx);
    }
    void Chunk::updateNeighborInfo(int idx)
    {
        int maxIdx = idx + (CHUNK_SIZE * CHUNK_SIZE);
        for (int i=idx; i<maxIdx; i++)
        {
            Block* currBlock = blocks[i];
            updateNeighbors(coords, currBlock);
            {
                std::lock_guard<std::mutex> lock(mutex);
                elementCount += getElementSize(currBlock);
            }
        }
    }
    void Chunk::findNeighbors()
    {
        vboSize = numBlocks * getVerticesCount();
        elementCount = 0;
        futures.clear();
        for ( int i=0; i<numBlocks; i+=(CHUNK_SIZE * CHUNK_SIZE) )
        {
            futures.emplace_back(
                pool.enqueue(&Chunk::updateNeighborInfo, this, i)
            );
        }
        for (auto& future : futures)
        {
            future.get();
        }

    }
    void Chunk::initBuffers()
    {
        futures.clear();
        Block* currBlock;
        vertexBufferData = new float[vboSize];
        elementBuffer = new unsigned int[elementCount];
        int vIdx = 0,
            eIdx = 0,
            currIdx = 0;

        for ( int i=0; i<numBlocks; i++ )
        {
            currBlock = blocks[i];
            futures.emplace_back(
                pool.enqueue(&Chunk::fillBuffers, this, currBlock, vIdx, eIdx, currIdx)
            );
            currIdx += 6 * 4;
            eIdx += getElementSize(currBlock);
            vIdx += getVerticesCount();
        }
        for (auto& future : futures)
        {
            future.get(); // Wait for all tasks to complete
        }
        initVAO();
    }

    void Chunk::initVAO()
    {
        // Init VAO and VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
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
    }
    void Chunk::drawChunk() const
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLint) elementCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}