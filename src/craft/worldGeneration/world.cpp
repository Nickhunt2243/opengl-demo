#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif

#define VISIBLE_CHUNKS 21                       // must be odd.
#define MAX_STORED_CHUNKS (VISIBLE_CHUNKS + 3) // must be odd.
#define CHUNK_BOUNDS ((VISIBLE_CHUNKS - 1) / 2)

#include "world.hpp"

namespace Craft
{
    World::World(
        Engine::Window* window,
        Engine::Program* program,
        Engine::Program* worldProgram,
        unsigned int width,
        unsigned int height
    )
        : coords{}
        , program{program}
        , worldProgram{worldProgram}
        , timer()
        , player{&timer, window, program, worldProgram, width, height, &coords}
        , sun{worldProgram}
    {
        updateChunkBounds();
    }
    void World::updateChunkBounds()
    {
        chunkStartX = (int) player.originChunk.x - (CHUNK_BOUNDS);
        chunkStartZ = (int) player.originChunk.z - (CHUNK_BOUNDS);
        chunkEndX = chunkStartX + (VISIBLE_CHUNKS);
        chunkEndZ = chunkStartZ + (VISIBLE_CHUNKS);
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
        Coordinate2D<int> currChunkCoord{x, z};
        {
            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks[currChunkCoord] = new Chunk(program, x, z, &coords, &coordsMutex);
        }
        chunks[currChunkCoord]->initChunk();
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
                if (chunks.find({x, z}) == chunks.end())
                {
                    initChunk(x, z);
                }
            }
        }
        for (int x=chunkStartX;x<chunkEndX; x++)
        {
            for (int z = chunkStartZ; z < chunkEndZ; z++)
            {
                chunks[{x, z}]->updateNeighborInfo();
            }
        }
        if (!player.initPlayer())
        {
            return false;
        }

        sun.initSun();

        timer.resetTimer();

        return true;
    }

    bool World::updateWorld()
    {
        // Derive whether the chunk the player is on has changed.
        Coordinate2D<int> directionDiff = player.updatePlayer();
        if (directionDiff == failureCoord)
        {
            return false;
        }
        if (directionDiff.x != 0 || directionDiff.z != 0)
        {
            updateChunkBounds();
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
                    for (int x = actualChunkStartX; x < actualChunkEndX; x++)
                    {
                        for (int z = actualChunkStartZ; z < actualChunkEndZ; z++)
                        {
                            // Create new Chunks
                            Coordinate2D<int> currChunkCoord{x, z};
                            if (chunks.find(currChunkCoord) == chunks.end())
                            {
                                initChunk((int) currChunkCoord.x, (int) currChunkCoord.z);
                            }
                        }
                    }
                    for (int x = actualChunkStartX; x < actualChunkEndX; x++)
                    {
                        for (int z = actualChunkStartZ; z < actualChunkEndZ; z++)
                        {
                            Coordinate2D<int> currChunkCoord{x, z};

                            if (directionDiff.x > 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkRight(
                                        &chunks[currChunkCoord]->blockCoords);
                            }
                            else if (directionDiff.x < 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkLeft(
                                        &chunks[currChunkCoord]->blockCoords);
                            }
                            else if (directionDiff.z > 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkFront(
                                        &chunks[currChunkCoord]->blockCoords);
                            }
                            else if (directionDiff.z < 0)
                            {
                                chunks[currChunkCoord - directionDiff]->updateChunkBack(
                                        &chunks[currChunkCoord]->blockCoords);
                            }
                            chunks[currChunkCoord]->updateNeighborInfo();
                        }
                    }
                })
            );


            // This method of deletion is slower, but safer, in my
            // opinion, than calculating which chunks we should delete.
            int playerOriginX = player.originChunk.x;//round(player.playerX / 16.0f);
            int playerOriginZ = player.originChunk.z;//round(player.playerZ / 16.0f);
            auto chunkIter= chunks.begin();
            int diff = MAX_STORED_CHUNKS / 2;
            while (chunkIter != chunks.end())
            {
                Coordinate2D<int> currCoord = chunkIter->first;
                if (
                        currCoord.x < playerOriginX - (diff) ||
                        currCoord.x > playerOriginX + (diff) ||
                        currCoord.z < playerOriginZ - (diff) ||
                        currCoord.z > playerOriginZ + (diff)
                    )
                {
                    {
                        std::lock_guard<std::mutex> lock(chunkMutex);
                        delete chunks[currCoord];
                        chunkIter = chunks.erase(chunkIter);
                    }
                    continue;
                }
                chunkIter++;
            }
        }

        // update sun position.
        sun.updateSun();

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
        timer.incFrames();
        for (int x=chunkStartX;x<chunkEndX; x++)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z++)
            {
                Coordinate2D<int> currChunkCoord{x, z};
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
                    if (chunks.find(currChunkCoord) != chunks.end()) {
                        chunks[currChunkCoord]->drawChunk(sun.getTime());
                    }
                }
            }
        }

        sun.drawLight((float) player.getWorldX(), (float) player.entityY, (float) player.getWorldZ());
//        std::cout << "FPS: " << timer.getFPS() << std::endl;

        return true;
    }
}