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
            Coordinate2D<int> chunkPos,
            std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords,
            std::mutex* coordsMutex
    )
        : coords{coords}
        , chunkPos(chunkPos)
        , coordsMutex{coordsMutex}
        , chunkIdx{((findChunkIdx(chunkPos.x) * TOTAL_CHUNK_WIDTH) + findChunkIdx(chunkPos.z))}
    {};
    Chunk::~Chunk() = default;
    void Chunk::initChunk(NeighborInfo* visibility, Textures* textures)
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

        int blockIdx, idx, yHeightFinal;
        Coordinate<int> baseCoord{0, 0, 0};
        Coordinate<int> worldCoord{chunkPos.x * 16, 0, chunkPos.z * 16};
        BlockTexture texture{};
        for (int xIdx=0; xIdx<CHUNK_WIDTH; xIdx++)
        {
            baseCoord.x = xIdx;
            worldCoord.z = chunkPos.z * 16;
            idx = xIdx;
            int chunkOffset = chunkIdx * BLOCKS_IN_CHUNK;
            for (int zIdx=0; zIdx<CHUNK_WIDTH; zIdx++)
            {
                baseCoord.z = zIdx;
                worldCoord.y = 0;
                yHeightFinal = yHeights[idx];
                for (int yIdx=0; yIdx<yHeightFinal; yIdx++)
                {
                    blockIdx = (yIdx * CHUNK_SIZE) + (zIdx * CHUNK_WIDTH) + xIdx;
                    baseCoord.y = yIdx;
                    blockType = yIdx < yHeightFinal - 3 ? BlockType::STONE : BlockType::GRASS;
                    blocksMap.insert({baseCoord, Block(blockType)});
                    texture = textures->textureMapping.at(blockType);

                    appendAllCoordInfo(visibility, chunkOffset + blockIdx, texture);
                    worldCoord.y += 1;
                }
                idx += CHUNK_WIDTH;
                worldCoord.z += 1;
            }
            worldCoord.x += 1;
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

        int idx = (chunkIdx * BLOCKS_IN_CHUNK) + (blockPos.y * CHUNK_SIZE) + (blockPos.z * CHUNK_WIDTH) + blockPos.x;
        visibility[idx].sideData = 0;
    }
    void Chunk::createBlock(Coordinate<int> blockPos, Textures* textures, NeighborInfo* visibility)
    {
        BlockType blockType = BlockType::STONE;
        auto newBlock = Block(blockType);
        int idx = (chunkIdx * BLOCKS_IN_CHUNK) + (blockPos.y * CHUNK_SIZE) + (blockPos.z * CHUNK_WIDTH) + blockPos.x;
        appendAllCoordInfo(visibility, idx, textures->textureMapping.at(blockType));
        visibility[idx].sideData |= 0x0ff;
        blocksMap.emplace(blockPos, newBlock);
    }
}