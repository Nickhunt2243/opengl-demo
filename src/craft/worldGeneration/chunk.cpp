#include <string>
#include <unordered_set>
#include <sstream>
#include <mutex>
#include <random>
#include <immintrin.h>

#include "chunk.hpp"
#include "../../helpers/timer.hpp"
#include "../../helpers/noise.hpp"

namespace Craft
{
    Textures* Chunk::textures = nullptr;

    Chunk::Chunk(
            Engine::Program* blockProgram,
            int x, int z,
            std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords,
            std::mutex* coordsMutex
    )
        : blockProgram{blockProgram}
        , coords{coords}
        , chunkPos(x, z)
        , coordsMutex{coordsMutex}
    {};
    Chunk::~Chunk()
    {
        blockProgram->useProgram();
        delete[] vertexBufferData;
        delete[] elementBuffer;
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

    void Chunk::initChunk(NeighborInfo* visibility) {
        BlockType blockType = BlockType::GRASS;

        std::mt19937 gen(44); // Mersenne Twister generator
        // Create a distribution for integers in the range [min, max]
        std::uniform_int_distribution<> dis(1, 100);
        // Generate a random number
        float random_amplitude = (float) dis(gen) / 10.0f;
        float random_frequency = (float) dis(gen) / 100.0f;
        float invScale = 1.0f / 7.0f;
        float fractalNoiseScale = 100.0f / 7;
        Craft::Noise noiseGenerator(44);

        auto chunkBaseX = (float) (chunkPos.x * 16);
        auto chunkBaseZ = (float) (chunkPos.z * 16);
        int yHeights[CHUNK_SIZE];
        std::memset(yHeights, 0, CHUNK_SIZE);
        Engine::Timer timer{};
        timer.startStopWatch();

        __m256 xWorld1 = _mm256_set_ps(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m256 xWorld2 = _mm256_set_ps(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        __m256 xScalar = _mm256_set1_ps(chunkBaseX);
        xWorld1 = _mm256_add_ps(xScalar, xWorld1);
        xWorld1 = _mm256_mul_ps(xWorld1, _mm256_set1_ps(invScale));
        xWorld2 = _mm256_add_ps(xScalar, xWorld2);
        xWorld2 = _mm256_mul_ps(xWorld2, _mm256_set1_ps(invScale));

        for (int z = 0; z<CHUNK_WIDTH; z++)
        {
            int idx1 = z * CHUNK_WIDTH;
            int idx2 = idx1 + (CHUNK_WIDTH / 2);
            __m256 currFractalNoise1 = noiseGenerator.__m256_fractalNoise_ps(
                    xWorld1, _mm256_set1_ps(fractalNoiseScale), _mm256_set1_ps((chunkBaseZ + z) * invScale),
                    10, 0.25f, random_amplitude, random_frequency
                );
            __m256 currFractalNoise2 = noiseGenerator.__m256_fractalNoise_ps(
                    xWorld2, _mm256_set1_ps(fractalNoiseScale), _mm256_set1_ps((chunkBaseZ + z) * invScale),
                    10, 0.25f, random_amplitude, random_frequency
            );

            currFractalNoise1 = _mm256_add_ps(_mm256_mul_ps(currFractalNoise1, _mm256_set1_ps(7.0f)), _mm256_set1_ps(CHUNK_BASE_HEIGHT));
            currFractalNoise2 = _mm256_add_ps(_mm256_mul_ps(currFractalNoise2, _mm256_set1_ps(7.0f)), _mm256_set1_ps(CHUNK_BASE_HEIGHT));
            _mm256_storeu_epi32((int*)(yHeights + idx1), _mm256_cvtps_epi32(currFractalNoise1));
            _mm256_storeu_epi32((int*)(yHeights + idx2), _mm256_cvtps_epi32(currFractalNoise2));
        }

        int offsetIdx, idx, yHeightFinal;
        Coordinate<int> baseCoord{0, 0, 0};
        for (int xIdx=0; xIdx<CHUNK_WIDTH; xIdx++)
        {
            baseCoord.x = xIdx;
            idx = xIdx;
            for (int zIdx=0; zIdx<CHUNK_WIDTH; zIdx++)
            {
                baseCoord.z = zIdx;
                yHeightFinal = yHeights[idx];
                for (int yIdx=0; yIdx<yHeightFinal; yIdx++)
                {
                    offsetIdx = (yIdx * CHUNK_SIZE) + (zIdx * CHUNK_WIDTH) + xIdx;
                    visibility[offsetIdx].setNeighbor(1);
                    baseCoord.y = yIdx;
                    blockType = yIdx < yHeightFinal - 3 ? BlockType::STONE : BlockType::GRASS;
                    blocksMap.emplace(baseCoord, Block(blockType));
                }
                idx += CHUNK_WIDTH;
            }
        }
        {
            std::lock_guard<std::mutex> lock(*coordsMutex);
            coords->insert({chunkPos, &blocksMap});
        }
    }
    void Chunk::initVisibility(NeighborInfo* visibility)
    {
        int idx = 0;
        for (auto blockInfo: blocksMap)
        {
            idx = (blockInfo.first.y * CHUNK_SIZE) + (blockInfo.first.z * CHUNK_WIDTH) + (blockInfo.first.x);
            visibility[idx].data = 1;
        }
    }

    void Chunk::deleteBlock(Coordinate<int> blockPos, NeighborInfo* visibility)
    {
        // Delete the block from the blocksMap hashmap
        if (blocksMap.find(blockPos) == blocksMap.end())
        {
            return;
        }
        blocksMap.erase(blockPos);

        int idx = (blockPos.y * CHUNK_SIZE) + (blockPos.z * CHUNK_WIDTH) + blockPos.x;

        visibility[idx].setNeighbor(0);
    }
    void Chunk::createBlock(Coordinate<int> blockPos, NeighborInfo* visibility)
    {
        BlockType blockType = BlockType::STONE;
        auto newBlock = Block(blockType);
        {
            std::lock_guard<std::mutex> lock(blocksMutex);
            blocksMap.emplace(blockPos, newBlock);
        }
        int idx = (blockPos.y * CHUNK_SIZE) + (blockPos.z * CHUNK_WIDTH) + blockPos.x;

        // Construct neighbor info:
        BlockInfo y_maxCoord = getBlockInfo({blockPos.x, blockPos.y + 1, blockPos.z}, chunkPos);
        BlockInfo y_minCoord = getBlockInfo({blockPos.x, blockPos.y - 1, blockPos.z}, chunkPos);
        BlockInfo x_maxCoord = getBlockInfo({blockPos.x + 1, blockPos.y, blockPos.z}, chunkPos);
        BlockInfo x_minCoord = getBlockInfo({blockPos.x - 1, blockPos.y, blockPos.z}, chunkPos);
        BlockInfo z_maxCoord = getBlockInfo({blockPos.x, blockPos.y, blockPos.z + 1}, chunkPos);
        BlockInfo z_minCoord = getBlockInfo({blockPos.x, blockPos.y, blockPos.z - 1}, chunkPos);

        int neighborInfo = 1; // set 1 bit to true because block exists.
        neighborInfo += (*coords)[y_maxCoord.chunk]->find(y_maxCoord.block) == (*coords)[y_maxCoord.chunk]->end() ? 2 : 0;
        neighborInfo += (*coords)[y_minCoord.chunk]->find(y_minCoord.block) == (*coords)[y_minCoord.chunk]->end() ? 4 : 0;
        neighborInfo += (*coords)[x_maxCoord.chunk]->find(x_maxCoord.block) == (*coords)[x_maxCoord.chunk]->end() ? 8 : 0;
        neighborInfo += (*coords)[x_minCoord.chunk]->find(x_minCoord.block) == (*coords)[x_minCoord.chunk]->end() ? 16 : 0;
        neighborInfo += (*coords)[z_maxCoord.chunk]->find(z_maxCoord.block) == (*coords)[z_maxCoord.chunk]->end() ? 32 : 0;
        neighborInfo += (*coords)[z_minCoord.chunk]->find(z_minCoord.block) == (*coords)[z_minCoord.chunk]->end() ? 64 : 0;

        visibility[idx].setNeighbor(neighborInfo);
    }
    void Chunk::initElementBuffer(NeighborInfo* visibility)
    {
        std::lock_guard<std::mutex> lock(blocksMutex);
        elementCount = 0;
        for (auto blockData: blocksMap)
        {
            Coordinate<int> relCoord = blockData.first;
            int idx = (relCoord.y * CHUNK_SIZE) + (relCoord.z * CHUNK_WIDTH) + (relCoord.x);
            int currSize = visibility[idx].sum() * 6;

            elementCount += currSize;
        }

        delete[] elementBuffer;
        elementBuffer = new uint32_t[elementCount];

        int eIdx = 0,
            currBlocksVerticesIdx = 0;
        for (auto blockData: blocksMap)
        {
            Coordinate<int> relCoord = blockData.first;
            int idx = (relCoord.y * CHUNK_SIZE) + (relCoord.z * CHUNK_WIDTH) + relCoord.x;
            NeighborInfo info = visibility[idx];
            fillElementBufferData(elementBuffer, eIdx, currBlocksVerticesIdx, info);
            currBlocksVerticesIdx += 6 * 4;
            eIdx += info.sum() * 6;
        }
        needToInitElements = true;
    }
    void Chunk::initBufferData()
    {
        std::lock_guard<std::mutex> lock(blocksMutex);
        vboSize = (int) blocksMap.size() * getVerticesCount();
        delete[] vertexBufferData;
        vertexBufferData = new int[vboSize];
        int vIdx = 0;
        for (auto blockData: blocksMap)
        {
            fillVerticesBufferData(blockData.first, blockData.second, vertexBufferData, vIdx, textures->textureMapping[blockData.second.type]);
            vIdx += getVerticesCount();
        }
        isReadyToInitVAO = true;
    }
    void Chunk::initEBO()
    {
        if (EBO == 0)
            glGenBuffers(1, &EBO);
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
        glVertexAttribIPointer(1, 1, GL_INT, stride, (void*) (1 * sizeof(int)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void Chunk::drawChunk(Time gameTime)
    {
        blockProgram->useProgram();
        if (isReadyToInitVAO)
        {
            initVAO();
        }
        if (VAO != 0 && needToInitElements) {
            initEBO();
        }
        if (canDrawChunk) {
            glBindVertexArray(VAO);
            float x = ((float) gameTime.hours * M_PI / 12) + M_PI;
            float cosX = cos(x);

            float newLightLevelEq1 = 7 * cosX + 5;
            float newLightLevelEq2 = 4 * abs(cosX);

            setFloat(blockProgram->getProgram(), "u_DefaultLightLevel", newLightLevelEq1 + newLightLevelEq2);
            setVec2(blockProgram->getProgram(), "u_ChunkPos", chunkPos * 16);
            glDrawElements(GL_TRIANGLES, (GLint) elementCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
}