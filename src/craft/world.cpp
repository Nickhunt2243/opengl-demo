//
// Created by admin on 6/8/2024.
//


#define CHUNK_WIDTH 5
#define VISIBLE_CHUNKS 3
#define CHUNK_BOUNDS ((VISIBLE_CHUNKS - 1) / 2)
#define NUM_CHUNKS (CHUNK_WIDTH * CHUNK_WIDTH) /// 7 x 7

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
        , player{window, program, width, height, &coords}
    {
        for (int i = 0; i<NUM_CHUNKS; i++)
        {
            chunks.push_back(new Chunk(&coords, &pool, &futures, &mutex));
        }
    }
    World::~World()
    {
        for (int i = 0; i < NUM_CHUNKS; i++) {
            delete chunks[i];
        }
        delete textures;
    }
    bool World::initWorld()
    {
        // init world
        textures = new Craft::Textures();
        textures->initTextures(program->getProgram());
        Craft::Chunk::textures = textures;

        int idx = 0;
        for (int x = -CHUNK_BOUNDS; x<=CHUNK_BOUNDS; x++)
        {
            for (int z = -CHUNK_BOUNDS; z<=CHUNK_BOUNDS; z++)
            {
                futures.emplace_back(
                        pool.enqueue([this, idx, x, z]()
                                     {
                                         chunks[idx]->initChunk(x, z);
                                     }
                        )
                );
                idx++;
            }
        }
        for (auto& future : futures)
        {
            future.get(); // Wait for all tasks to complete
        }
        idx = 0;
        for (int i = -CHUNK_BOUNDS; i<=CHUNK_BOUNDS; i++)
        {
            for (int j = -CHUNK_BOUNDS; j<=CHUNK_BOUNDS; j++)
            {
                chunks[idx]->initBufferData();
                idx++;
            }
        }
        std::cout << "Initializing Camera." << std::endl;
        if (!player.initPlayer())
        {
            return false;
        }

        return true;
    }

    bool World::updateWorld()
    {
        // Get player x and z

        // Check if the player is in a new origin chunk

        // If player is, then initialize the additional chunks


        if (player.updatePlayer())
        {
            return false;
        }

        return true;
    }

    bool World::drawWorld()
    {
        float playerX = player.playerX;
        float playerZ = player.playerZ;

        for (int x=-CHUNK_BOUNDS;x<CHUNK_BOUNDS; x++)
        {
            for (int z=-CHUNK_BOUNDS;z<CHUNK_BOUNDS; z++)
            {

            }
        }

        // Display State
        for (int i=0; i<NUM_CHUNKS; i++) {
            chunks[i]->drawChunk();
        }

        return true;
    }
}