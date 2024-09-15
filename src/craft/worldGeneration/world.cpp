
#include "world.hpp"
#include "../misc/globals.hpp"

namespace Craft
{
    World::World(
        Engine::Window* window,
        Engine::Program* blockProgram,
        Engine::Program* worldProgram,
        Engine::Compute* neighborCompute,
        uint32_t width,
        uint32_t height
    )
        : coords{}
        , window{window}
        , blockProgram{blockProgram}
        , worldProgram{worldProgram}
        , neighborCompute{neighborCompute}
        , timer()
        , player{&timer, window, blockProgram, worldProgram, width, height, &coords, &coordsMutex}
        , sun{worldProgram}
    {
        updateChunkBounds();
    }
    BlockInfo calcBlockData(Coordinate<int> blockData)
    {
        Coordinate2D<int> chunkPos{(int) floor((float) blockData.x / 16.0f), (int) floor((float) blockData.z / 16.0f)};
        Coordinate<int> blockRelPos{blockData.x - (chunkPos.x * 16), blockData.y, blockData.z - (chunkPos.z * 16)};

        return {blockRelPos, chunkPos};
    }
//    void World::updateNeighbors(
//            Coordinate<int> blockPos,
//            Coordinate2D<int> chunkPos,
//            bool drawBlock
//        )
//    {
////        std::unordered_set<BlockInfo> chunksToUpdate{};
//        // Construct neighbor info:
//        std::vector<Coordinate<int>> blockOffsets = {
//                {0, -1, 0},
//                {0, 1, 0},
//                {-1, 0, 0},
//                {1, 0, 0},
//                {0, 0, -1},
//                {0, 0, 1}
//        };
//        int sideIdx =0;
//        for (auto coordOffset: blockOffsets)
//        {
//            sideIdx++;
//            BlockInfo info = getBlockInfo((Coordinate<int>{blockPos.x, blockPos.y, blockPos.z} + coordOffset), chunkPos);
//            int chunkIdxX = (((info.chunk.x + RENDER_DISTANCE) % TOTAL_CHUNK_WIDTH) + TOTAL_CHUNK_WIDTH) % TOTAL_CHUNK_WIDTH;
//            int chunkIdxZ = (((info.chunk.z + RENDER_DISTANCE) % TOTAL_CHUNK_WIDTH) + TOTAL_CHUNK_WIDTH) % TOTAL_CHUNK_WIDTH;
//
//            int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
//            int blockIdx = (info.block.y * CHUNK_SIZE) + (info.block.z * CHUNK_WIDTH) + info.block.x;
//            NeighborInfo neighbor = chunkSSBOPointer[chunkIdx].blockVisibility[blockIdx];
//            if (!neighbor.blockExists())
//            {
//                continue;
//            }
//            neighbor[sideIdx] = drawBlock;
//            chunkSSBOPointer[chunkIdx].blockVisibility[blockIdx] = neighbor;
//        }
//    }
    int findChunkIdx(int coord)
    {
        if ((coord + RENDER_DISTANCE) < 0)
        {
            return ((((((coord + RENDER_DISTANCE) * -1) % TOTAL_CHUNK_WIDTH) * -1) + TOTAL_CHUNK_WIDTH) % TOTAL_CHUNK_WIDTH);
        }
        else
        {
            return (((coord + RENDER_DISTANCE) % TOTAL_CHUNK_WIDTH) + TOTAL_CHUNK_WIDTH) % TOTAL_CHUNK_WIDTH;
        }
    }
    // Callback function to handle mouse button events
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        auto userPointerData = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(window));
        World* world = userPointerData->world;
        if (world == nullptr) return;
        std::unordered_set<Coordinate2D<int>> chunksToUpdate{};
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            std::cout << "Left Press" << std::endl;
            // Handle left mouse button press
            if (world->player.lookAtBlock != nullptr) {
                Coordinate<int> blockToDelete = *world->player.lookAtBlock;
                BlockInfo info = calcBlockData(blockToDelete);

                if (world->chunks.find(info.chunk) != world->chunks.end())
                {
                    std::lock_guard<std::mutex> lock(world->chunkMutex);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBO);
                    if (world->chunkSSBOPointer == nullptr)
                    {
                        std::cerr << "Failed to retrieve SSBO Data." << std::endl;
                        return;
                    }
                    int chunkIdxX = findChunkIdx(info.chunk.x);
                    int chunkIdxZ = findChunkIdx(info.chunk.z);

                    int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
                    world->chunks[info.chunk]->deleteBlock(info.block, world->chunkSSBOPointer[chunkIdx].blockVisibility);
                    world->needsToUpdateNeighbors = true;
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
                }
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            // add block
            if (world->player.lookAtBlock != nullptr && !world->player.playerIntersectsBlock())
            {
                Coordinate<int> blockToCreate = world->player.getNextLookAtBlock();
                BlockInfo info = calcBlockData(blockToCreate);

                if (world->chunks.find(info.chunk) != world->chunks.end())
                {
                    std::lock_guard<std::mutex> lock(world->chunkMutex);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBO);
                    if (world->chunkSSBOPointer == nullptr)
                    {
                        std::cerr << "Failed to retrieve SSBO Data." << std::endl;
                        return;
                    }
                    int chunkIdxX = findChunkIdx(info.chunk.x);
                    int chunkIdxZ = findChunkIdx(info.chunk.z);
                    int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
                    world->chunks[info.chunk]->createBlock(info.block, world->textures, world->chunkSSBOPointer[chunkIdx].blockVisibility);
                    world->needsToUpdateNeighbors = true;
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
                }
            }
            else
            {
                return;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            std::cout << "Mouse button released." << std::endl;
            // Handle mouse button release
        }
    }

    void World::updateChunkBounds()
    {
        chunkStartX = (int) player.originChunk.x - (RENDER_DISTANCE);
        chunkStartZ = (int) player.originChunk.z - (RENDER_DISTANCE);
        chunkEndX = (int) player.originChunk.x + (RENDER_DISTANCE) + 1;
        chunkEndZ = (int) player.originChunk.z + (RENDER_DISTANCE) + 1;
        minChunkCoord = {chunkStartX, chunkStartZ};
    }
    World::~World()
    {
        if (chunkSSBOPointer)
        {
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }

        delete[] blockCounts;
        delete[] chunkFirst;
        delete textures;
        delete userPointer;
        if (SSBO != 0)
        {
            glDeleteVertexArrays(1, &(SSBO));
        }
        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &(VAO));
        }
        if (IBO != 0)
        {
            glDeleteVertexArrays(1, &(IBO));
        }
    }
    void World::initChunk(int x, int z, NeighborInfo* visibility)
    {
        Coordinate2D<int> currChunkCoord{x, z};
        {
            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks.emplace(currChunkCoord, std::make_unique<Chunk>(blockProgram, x, z, &coords, &coordsMutex));
        }
        chunks[currChunkCoord]->initChunk(visibility, textures);
    }
    void World::initBuffers()
    {
        // Init VAO, IBO and,
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &IBO);
        glGenBuffers(1, &SSBO);
        glGenBuffers(1, &VBO);
        // Bind VAO
        glBindVertexArray(VAO);

        float vertexData[3];
        vertexData[0] = 0.0f;
        vertexData[1] = 0.0f;
        vertexData[2] = 0.0f;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), vertexData, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Setup Indirect Buffer Object
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, TOTAL_MAX_CHUNKS * sizeof(DrawArraysIndirectCommand), nullptr, GL_DYNAMIC_DRAW);

        // Bind and initialize the storage for the SSBO.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO), nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        GLuint chunkInfoIdx = 0;
        // Define the SSBO for the neighbor compute shader.
        neighborCompute->useCompute();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunkInfoIdx, SSBO);

        // Define the SSBO for the block shaders.
        blockProgram->useProgram();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunkInfoIdx, SSBO);

        // Map the buffer for further updates.
        chunkSSBOPointer = (ChunkInfoSSBO*) glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER, 0, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        memset(chunkSSBOPointer, 0, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO));
    }
    bool World::initWorld()
    {
        // init world
        textures = new Craft::Textures();
        textures->initTextures(blockProgram->getProgram());

        initBuffers();

        if (chunkSSBOPointer == nullptr)
        {
            std::cerr << "Failed to retrieve SSBO Data." << std::endl;
            return false;
        }

        auto drawCommands = (DrawArraysIndirectCommand*) glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_WRITE_ONLY);

        auto chunkOffset = (GLsizei) (BLOCKS_IN_CHUNK);
        GLsizei chunkIdxX, chunkIdxZ, chunkIdx;
        for (int x=chunkStartX;x<chunkEndX; x++)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z++)
            {
                if (chunks.find({x, z}) == chunks.end())
                {
                    chunkIdxX = findChunkIdx(x);
                    chunkIdxZ = findChunkIdx(z);
                    chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
                    initChunk(x, z, chunkSSBOPointer[chunkIdx].blockVisibility);

                    drawCommands[chunkIdx].first = 0;
                    drawCommands[chunkIdx].count = 1;
                    drawCommands[chunkIdx].instanceCount = BLOCKS_IN_CHUNK;
                    drawCommands[chunkIdx].baseInstance = chunkIdx * chunkOffset;
                }
            }
        }

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
        calcNeighborInfo();
        if (!player.initPlayer())
        {
            return false;
        }

        sun.initSun();

        // Set GLFW User pointer variables
        userPointer = new GLFWUserPointer{};
        userPointer->camera = player.getCamera();
        userPointer->world = this;
        // Set the mouse button callback
        glfwSetWindowUserPointer(window->getWindow(), userPointer);
        glfwSetMouseButtonCallback(window->getWindow(), mouse_button_callback);

        timer.resetTimer();
        return true;
    }
    void World::calcNeighborInfo()
    {
        // Use the compute shader blockProgram
        neighborCompute->useCompute();
        int worldWidth = (TOTAL_CHUNK_WIDTH * CHUNK_WIDTH);
        setInt(neighborCompute->getProgram(), "u_WorldWidth", worldWidth);
        setInt(neighborCompute->getProgram(), "u_numChunks", TOTAL_CHUNK_WIDTH);
        setInt(neighborCompute->getProgram(), "u_renderDistance", RENDER_DISTANCE);
        setiVec2(neighborCompute->getProgram(), "u_minChunkCoords", minChunkCoord);
        // Dispatch the compute shader
        glDispatchCompute(worldWidth, CHUNK_HEIGHT, worldWidth);
        // Make sure the compute shader has finished before using the data
        glFinish();
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    void World::translateChunkSSBOData(Coordinate2D<int> directionDiff) const
    {
        Coordinate2D<int> newChunkCoord = player.originChunk + (directionDiff * RENDER_DISTANCE);
        if (directionDiff.x != 0)
        {
            int x = findChunkIdx(newChunkCoord.x);
            for (int z=0; z<TOTAL_CHUNK_WIDTH; z++)
            {
                int chunkIdx = (x * TOTAL_CHUNK_WIDTH) + z;
                memset(&chunkSSBOPointer[chunkIdx], 0, sizeof(ChunkInfoSSBO));
            }
        }
        else if (directionDiff.z != 0)
        {
            int z = findChunkIdx(newChunkCoord.z);
            for (int x=0; x<TOTAL_CHUNK_WIDTH; x++)
            {
                int chunkIdx = (x * TOTAL_CHUNK_WIDTH) + z;
                memset(&chunkSSBOPointer[chunkIdx], 0, sizeof(ChunkInfoSSBO));
            }
        }
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    void World::updateChunksLoaded(Coordinate2D<int> directionDiff)
    {
        updateChunkBounds();

        translateChunkSSBOData(directionDiff);

        futures.emplace_back(
            pool.enqueue([this]() {
                for (int x=chunkStartX; x<chunkEndX; x++)
                {
                    for (int z=chunkStartZ; z<chunkEndZ; z++)
                    {
                        // Create new Chunks
                        Coordinate2D<int> currChunkCoord{x, z};
                        if (chunks.find(currChunkCoord) == chunks.end())
                        {
                            int chunkIdxX = findChunkIdx(x);
                            int chunkIdxZ = findChunkIdx(z);
                            int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
                            initChunk(currChunkCoord.x, currChunkCoord.z, chunkSSBOPointer[chunkIdx].blockVisibility);
                        }
                    }
                }
                needsToUpdateNeighbors = true;
            })
        );

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        for (auto chunkIter = chunks.begin(); chunkIter != chunks.end(); )
        {
            if (
                    chunkIter->first.x < chunkStartX || chunkIter->first.x >= chunkEndX ||
                    chunkIter->first.z < chunkStartZ || chunkIter->first.z >= chunkEndZ
                )
            {
                coords.erase(chunkIter->first);
                chunkIter = chunks.erase(chunkIter);
            }
            else
            {
                chunkIter++;
            }
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
    }
    bool World::updateWorld()
    {
        Coordinate2D<int> directionDiff = player.updatePlayer();
        if (directionDiff == failureCoord) return false;
        if (directionDiff.x != 0 || directionDiff.z != 0)
        {
            updateChunksLoaded(directionDiff);
        }
        // update sun position.
        sun.updateSun();

        // Update Neighbor Info
        if (needsToUpdateNeighbors)
        {
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
            calcNeighborInfo();
            needsToUpdateNeighbors = false;
        }

        return true;
    }

    bool World::drawWorld() {
        blockProgram->useProgram();
        timer.incFrames();

        glBindVertexArray(VAO);
        float x = ((float) sun.getTime().hours * M_PI / 12) + M_PI;
        float cosX = cos(x);

        float newLightLevelEq1 = 7 * cosX + 5;
        float newLightLevelEq2 = 4 * abs(cosX);

        setFloat(blockProgram->getProgram(), "u_defaultLightLevel", newLightLevelEq1 + newLightLevelEq2);
        setInt(blockProgram->getProgram(), "u_numChunks", TOTAL_CHUNK_WIDTH);
        setInt(blockProgram->getProgram(), "u_renderDistance", RENDER_DISTANCE);
        setiVec2(blockProgram->getProgram(), "u_minChunkCoords", minChunkCoord);

        glMultiDrawArraysIndirect(GL_POINTS, nullptr, TOTAL_MAX_CHUNKS, 0);
        glBindVertexArray(0);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
        glFinish();
        sun.drawLight((float) player.getWorldX(), (float) player.entityY, (float) player.getWorldZ());
//        std::cout << "FPS: " << timer.getFPS() << std::endl;

        return true;
    }
}

// 7, 8, 6
// 4, 5, 3
// 1, 2, 0