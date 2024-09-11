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
    Chunk::Chunk(
            Engine::Program* blockProgram,
            int x, int z,
            std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords,
            std::mutex* coordsMutex, GLuint VBO
    )
        : blockProgram{blockProgram}
        , coords{coords}
        , chunkPos(x, z)
        , coordsMutex{coordsMutex}
        , VBO{VBO}
    {};
    void Chunk::initChunk(NeighborInfo* visibility)
    {
        BlockType blockType;

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
    void Chunk::initBufferData(Textures* textures, int chunkIdx)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        auto chunkByteOffset = (GLsizei) (BLOCKS_IN_CHUNK * sizeof(BlockVertexData));
        GLsizei chunkVBOIdx = chunkIdx * chunkByteOffset;

        auto blockData = (BlockVertexData*) glMapBufferRange(GL_ARRAY_BUFFER, chunkVBOIdx, chunkByteOffset, GL_MAP_WRITE_BIT);
        // Reset the blockData.
        if (blockData == nullptr)
        {
            std::cerr << "Failed to retrieve Buffer Data." << std::endl;
            return;
        }
        // Reset this range to 0 prior to writing to it.
        memset(blockData, 0, chunkByteOffset);
        vboSize = (int) blocksMap.size();

        int vIdx = 0;
        BlockTexture texture{};
        Coordinate<int> blockWorldCoord{chunkPos.x * 16, 0, chunkPos.z * 16};
        for (auto block: blocksMap)
        {
            texture = textures->textureMapping.at(block.second.type);
            appendAllCoordInfo(blockData[vIdx], {blockWorldCoord.x + block.first.x, block.first.y, blockWorldCoord.z + block.first.z}, texture);
            vIdx++;
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}