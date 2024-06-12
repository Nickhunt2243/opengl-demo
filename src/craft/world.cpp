//
// Created by admin on 6/8/2024.
//

#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif

#define VISIBLE_CHUNKS 5
#define MAX_STORED_CHUNKS (VISIBLE_CHUNKS + 4)
#define CHUNK_BOUNDS ((VISIBLE_CHUNKS - 1) / 2)
#define MAX_STORED_BOUNDS ((MAX_STORED_CHUNKS - 1) / 2)
#define NUM_CHUNKS (WORLD_WIDTH * WORLD_WIDTH) /// 7 x 7

#include "world.hpp"

namespace Craft
{
    World::World(
            Engine::Window* window,
            Engine::Program* program,
            unsigned int width,
            unsigned int height
    )
        : coords{}
        , program{program}
        , player{window, program, width, height, &coords, &coordsMutex}
        , playerOriginCoord(0, 0)
    {
        calcChunkBounds();
    }

    void World::calcChunkBounds()
    {
        float playerOriginX = round(player.playerX / 16.0f) * 16.0f;
        float playerOriginZ = round(player.playerZ / 16.0f) * 16.0f;
        playerOriginCoord = Coordinate2D{playerOriginX, playerOriginZ};
        chunkStartX = (int) playerOriginX - (CHUNK_BOUNDS * 16);
        chunkStartZ = (int) playerOriginZ - (CHUNK_BOUNDS * 16);
        chunkEndX = chunkStartX + (VISIBLE_CHUNKS * 16);
        chunkEndZ = chunkStartZ + (VISIBLE_CHUNKS * 16);
    }
    World::~World()
    {
        for (auto chunk: chunks)
        {
            delete chunk.second;
        }
        delete textures;
    }
    void World::initChunk(int x, int z)
    {
        Coordinate2D currChunkCoord{(float) x, (float) z};
        {
            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks[currChunkCoord] = new Chunk(&coords, &pool, &futures, &coordsMutex);
        }
        chunks[currChunkCoord]->initChunk(x, z);
        chunks[currChunkCoord]->initBufferData();
    }

    bool World::initWorld()
    {
        // init world
        textures = new Craft::Textures();
        textures->initTextures(program->getProgram());
        Craft::Chunk::textures = textures;
        for (int x=chunkStartX;x<chunkEndX; x+=16)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z+=16)
            {
                if (chunks.find({(float) x, (float) z}) == chunks.end())
                {
                    initChunk(x, z);
                }
            }
        }

        for (auto chunk: chunks)
        {
            chunk.second->initElementBuffer();
        }
        futures.clear();
        if (!player.initPlayer())
        {
            return false;
        }

        return true;
    }

    bool World::updateWorld()
    {
        // Get player x and z as the origin of a chunk
        float playerOriginX = round(player.playerX / 16.0f) * 16.0f;
        float playerOriginZ = round(player.playerZ / 16.0f) * 16.0f;
        // Check if the player is in a new origin chunk
        if (playerOriginCoord.x != playerOriginX || playerOriginCoord.z != playerOriginZ) {
            // If player is, then initialize the additional chunks
            calcChunkBounds();
            std::vector<Coordinate2D> chunksToAdd{};
            for (int x=chunkStartX;x<chunkEndX; x+=16)
            {
                for (int z=chunkStartZ;z<chunkEndZ; z+=16)
                {
                    Coordinate2D currChunkCoord{(float) x, (float) z};
                    if (chunks.find(currChunkCoord) == chunks.end())
                    {
                        chunksToAdd.push_back(currChunkCoord);
                    }
                }
            }
            futures.emplace_back(
                std::async(std::launch::async, [this, chunksToAdd]()
                {
                    for (auto coord: chunksToAdd) {
                        initChunk((int) coord.x, (int) coord.z);
                    }
                    for (auto coord: chunksToAdd) {
                        chunks[coord]->initElementBuffer();
                    }
                })
            );
        }

        auto minChunkX = (float) (playerOriginX - (MAX_STORED_BOUNDS * 16)),
             minChunkZ = (float) (playerOriginZ - (MAX_STORED_BOUNDS * 16)),
             maxChunkX = (float) (playerOriginX + (MAX_STORED_BOUNDS * 16)),
             maxChunkZ = (float) (playerOriginZ + (MAX_STORED_BOUNDS * 16));

        auto chunkIter = chunks.begin();
        while(chunkIter != chunks.end())
        {
            if (chunkIter->first.x < minChunkX || chunkIter->first.x > maxChunkX ||
                chunkIter->first.z < minChunkZ || chunkIter->first.z > maxChunkZ)
            {
                delete chunkIter->second;
                chunkIter = chunks.erase(chunkIter);
                continue;
            }
            chunkIter++;
        }
        if (player.updatePlayer())
        {
            return false;
        }

        futures.erase(
            std::remove_if(
                futures.begin(),
                futures.end(),
                []( const std::future<void>& future )
                    {
                        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
            ),
            futures.end()
        );

        return true;
    }

    bool World::drawWorld()
    {
        for (int x=chunkStartX;x<chunkEndX; x+=16)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z+=16)
            {
                Coordinate2D currChunkCoord{(float) x, (float) z};
                if (chunks.find(currChunkCoord) != chunks.end()) {
                    chunks[currChunkCoord]->drawChunk();
                }
            }
        }

        return true;
    }
}