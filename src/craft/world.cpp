#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif

#define VISIBLE_CHUNKS 15                      // must be odd.
#define MAX_STORED_CHUNKS (VISIBLE_CHUNKS + 3) // must be odd.
#define CHUNK_BOUNDS ((VISIBLE_CHUNKS - 1) / 2)

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

    Coordinate2D World::calcChunkBounds()
    {
        float playerOriginX = round(player.playerX / 16.0f);
        float playerOriginZ = round(player.playerZ / 16.0f);
        Coordinate2D newPlayerOrigin{playerOriginX, playerOriginZ};
        Coordinate2D diff = newPlayerOrigin - playerOriginCoord;
        playerOriginCoord = newPlayerOrigin;
        chunkStartX = (int) playerOriginX - (CHUNK_BOUNDS);
        chunkStartZ = (int) playerOriginZ - (CHUNK_BOUNDS);
        chunkEndX = chunkStartX + (VISIBLE_CHUNKS);
        chunkEndZ = chunkStartZ + (VISIBLE_CHUNKS);
        return diff;
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
            chunks[currChunkCoord] = new Chunk(program->getProgram(), (float) x, (float) z, &coords, &coordsMutex);
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
        for (int x=chunkStartX;x<chunkEndX; x++)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z++)
            {
                if (chunks.find({(float) x, (float) z}) == chunks.end())
                {
                    initChunk(x, z);
                }
            }
        }
        for (int x=chunkStartX;x<chunkEndX; x++) {
            for (int z = chunkStartZ; z < chunkEndZ; z++) {
                chunks[{(float) x, (float) z}]->updateNeighborInfo();
                chunks[{(float) x, (float) z}]->initElementBuffer();
            }
        }
        if (!player.initPlayer())
        {
            return false;
        }

        return true;
    }

    bool World::updateWorld()
    {
        // Get player x and z as the origin of a chunk
        float playerOriginX = round(player.playerX / 16.0f);
        float playerOriginZ = round(player.playerZ / 16.0f);
        // Check if the player is in a new origin chunk
        if (playerOriginCoord.x != playerOriginX || playerOriginCoord.z != playerOriginZ) {
            // If player is, then initialize the additional chunks
            Coordinate2D directionDiff = calcChunkBounds();
            int actualChunkStartX = chunkStartX,
                actualChunkEndX = chunkEndX,
                actualChunkStartZ = chunkStartZ,
                actualChunkEndZ = chunkEndZ;
            if (directionDiff.x > 0)
            {
                actualChunkStartX = chunkEndX - 1;
            }
            else if (directionDiff.x < 0)
            {
                actualChunkEndX = chunkStartX + 1;
            }
            else if (directionDiff.z > 0)
            {
                actualChunkStartZ = chunkEndZ - 1;
            }
            else if (directionDiff.z < 0)
            {
                actualChunkEndZ = chunkStartZ + 1;
            }

            futures.emplace_back(
                std::async(std::launch::async, [this, actualChunkStartX, actualChunkEndX, actualChunkStartZ, actualChunkEndZ, directionDiff]()
                {
                    std::lock_guard<std::mutex> lock(initOneSideChunksMutex);
                    for (int x=actualChunkStartX;x<actualChunkEndX; x++)
                    {
                        for (int z=actualChunkStartZ;z<actualChunkEndZ; z++)
                        {
                            // Create new Chunks
                            Coordinate2D currChunkCoord{(float) x, (float) z};
                            if (chunks.find(currChunkCoord) == chunks.end())
                            {
                                initChunk((int) currChunkCoord.x, (int) currChunkCoord.z);
                            }
                            if (directionDiff.x > 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkRight(&chunks[currChunkCoord]->blockCoords);
                            }
                            else if (directionDiff.x < 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkLeft(&chunks[currChunkCoord]->blockCoords);
                            }
                            else if (directionDiff.z > 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkFront(&chunks[currChunkCoord]->blockCoords);
                            }
                            else if (directionDiff.z < 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkBack(&chunks[currChunkCoord]->blockCoords);
                            }
                        }
                    }
                    for (int x=actualChunkStartX;x<actualChunkEndX; x++) {
                        for (int z = actualChunkStartZ; z < actualChunkEndZ; z++) {
                            chunks[{(float) x, (float) z}]->updateNeighborInfo();
                            chunks[{(float) x, (float) z}]->initElementBuffer();
                        }
                    }
                })
            );

            // This method of deletion is slower, but safer in my
            // opinion than calculating which chunks we should delete.
            auto chunkIter= chunks.begin();
            float diff = (int) (MAX_STORED_CHUNKS / 2);
            while (chunkIter != chunks.end())
            {
                Coordinate2D currCoord = chunkIter->first;
                if (
                        currCoord.x < playerOriginX - (diff) ||
                        currCoord.x > playerOriginX + (diff) ||
                        currCoord.z < playerOriginZ - (diff) ||
                        currCoord.z > playerOriginZ + (diff)
                    )
                {
                    delete chunks[currCoord];
                    chunkIter = chunks.erase(chunkIter);
                    continue;
                }
                chunkIter++;
            }
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
//        chunks[Coordinate2D{0, 0}]->drawChunk();
//        chunks[Coordinate2D{1, 0}]->drawChunk();
        for (int x=chunkStartX;x<chunkEndX; x++)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z++)
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