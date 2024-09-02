
#include "world.hpp"
#include "../misc/globals.hpp"

namespace Craft
{
    World::World(
        Engine::Window* window,
        Engine::Program* program,
        Engine::Program* worldProgram,
        Engine::Compute* neighborCompute,
        uint32_t width,
        uint32_t height
    )
        : coords{}
        , window{window}
        , program{program}
        , worldProgram{worldProgram}
        , neighborCompute{neighborCompute}
        , timer()
        , player{&timer, window, program, worldProgram, width, height, &coords, &coordsMutex}
        , sun{worldProgram}
    {
        updateChunkBounds();
    }
    BlockInfo calcBlockData(Coordinate<int> blockData)
    {
        Coordinate2D<int> blockChunkPos{0, 0};
        Coordinate<int> blockRelPos{blockData.x % 16, blockData.y, blockData.z % 16};
        BlockInfo info;

        blockRelPos.x = blockRelPos.x < 0 ? blockRelPos.x + 16 : blockRelPos.x;
        blockRelPos.z = blockRelPos.z < 0 ? blockRelPos.z + 16 : blockRelPos.z;

        blockChunkPos.x = (blockData.x - blockRelPos.x) / 16;
        blockChunkPos.z = (blockData.z - blockRelPos.z) / 16;

        info.chunk = blockChunkPos;
        info.block = blockRelPos;
        return info;
    }
    std::unordered_set<Coordinate2D<int>> World::updateNeighbors(
            Coordinate<int> blockPos,
            Coordinate2D<int> chunkPos,
            ChunkInfoSSBO* chunkInfoSSBO,
            bool drawBlock
        )
    {
        std::unordered_set<Coordinate2D<int>> chunksToUpdate{};
        // Construct neighbor info:
        std::vector<Coordinate<int>> blockOffsets = {
                {0, -1, 0},
                {0, 1, 0},
                {-1, 0, 0},
                {1, 0, 0},
                {0, 0, -1},
                {0, 0, 1}
        };
        int sideIdx =0;
        for (auto coordOffset: blockOffsets)
        {
            sideIdx++;
            BlockInfo info = getBlockInfo((Coordinate<int>{blockPos.x, blockPos.y, blockPos.z} + coordOffset), chunkPos);
            chunksToUpdate.insert(info.chunk);
            int chunkIdx = ((info.chunk.x - minChunkCoords.x) * MAX_STORED_CHUNKS) + (info.chunk.z - minChunkCoords.z);
            int blockIdx = (info.block.y * CHUNK_SIZE) + (info.block.z * CHUNK_WIDTH) + info.block.x;
            NeighborInfo neighbor = chunkInfoSSBO[chunkIdx].blockVisibility[blockIdx];
            if (!neighbor.blockExists())
            {
                continue;
            }
            neighbor[sideIdx] = drawBlock;
            chunkInfoSSBO[chunkIdx].blockVisibility[blockIdx] = neighbor;
        }

        return chunksToUpdate;
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
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->worldChunkInfoSSBOID);
                    if (world->chunkSSBOPointer == nullptr)
                    {
                        std::cerr << "Failed to retrieve SSBO Data." << std::endl;
                        return;
                    }
                    int chunkIdx = ((info.chunk.x - world->minChunkCoords.x) * MAX_STORED_CHUNKS) + info.chunk.z - world->minChunkCoords.z;
                    world->chunks[info.chunk]->deleteBlock(info.block, world->chunkSSBOPointer[chunkIdx].blockVisibility);

                    chunksToUpdate = world->updateNeighbors(info.block, info.chunk, world->chunkSSBOPointer, true);
                    for (auto chunk: chunksToUpdate)
                    {
                        chunkIdx = ((chunk.x - world->minChunkCoords.x) * MAX_STORED_CHUNKS) + chunk.z - world->minChunkCoords.z;
                        world->chunks[chunk]->initBufferData();
                        world->chunks[chunk]->initElementBuffer(world->chunkSSBOPointer[chunkIdx].blockVisibility);
                    }
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
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->worldChunkInfoSSBOID);
                    if (world->chunkSSBOPointer == nullptr)
                    {
                        std::cerr << "Failed to retrieve SSBO Data." << std::endl;
                        return;
                    }

                    int chunkIdx = ((info.chunk.x - world->minChunkCoords.x) * MAX_STORED_CHUNKS) + info.chunk.z - world->minChunkCoords.z;
                    world->chunks[info.chunk]->createBlock(info.block, world->chunkSSBOPointer[chunkIdx].blockVisibility);

                    chunksToUpdate = world->updateNeighbors(info.block, info.chunk, world->chunkSSBOPointer, false);
                    for (auto chunk: chunksToUpdate)
                    {
                        chunkIdx = ((chunk.x - world->minChunkCoords.x) * MAX_STORED_CHUNKS) + chunk.z - world->minChunkCoords.z;
                        world->chunks[chunk]->initBufferData();
                        world->chunks[chunk]->initElementBuffer(world->chunkSSBOPointer[chunkIdx].blockVisibility);
                    }
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
        chunkStartX = (int) player.originChunk.x - (CHUNK_BOUNDS);
        chunkStartZ = (int) player.originChunk.z - (CHUNK_BOUNDS);
        chunkEndX = chunkStartX + (VISIBLE_CHUNKS);
        chunkEndZ = chunkStartZ + (VISIBLE_CHUNKS);
        minChunkCoords = {chunkStartX, chunkStartZ};
    }
    World::~World()
    {
        glDeleteBuffers(1, &worldChunkInfoSSBOID);
        for (auto chunk: chunks)
        {
            delete chunk.second;
        }
        if (chunkSSBOPointer)
        {
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
        delete textures;
        delete userPointer;
    }
    void World::initChunk(int x, int z, NeighborInfo* visibility)
    {
        Engine::Timer timer{};

        timer.startStopWatch();
        float currTime = timer.lapStopWatch(), prevTime;

        Coordinate2D<int> currChunkCoord{x, z};
        {
            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks[currChunkCoord] = new Chunk(program, x, z, &coords, &coordsMutex);
            prevTime = currTime;
            currTime = timer.lapStopWatch();
            std::cout << "\tTime for create chunk: " << (currTime - prevTime) << " ms " << std::endl;
        }
        chunks[currChunkCoord]->initChunk(visibility);
        prevTime = currTime;
        currTime = timer.lapStopWatch();
        std::cout << "\tTime for init Chunk data: " << (currTime - prevTime) << " ms " << std::endl;
        chunks[currChunkCoord]->initVisibility(visibility);
        prevTime = currTime;
        currTime = timer.lapStopWatch();
        std::cout << "\tTime for init visibility: " << (currTime - prevTime) << " ms " << std::endl;
        chunks[currChunkCoord]->initBufferData();
        prevTime = currTime;
        currTime = timer.lapStopWatch();
        std::cout << "\tTime for init buffer: " << (currTime - prevTime) << " ms " << std::endl;
    }
    bool World::initWorld()
    {
        // init world
        textures = new Craft::Textures();
        textures->initTextures(program->getProgram());
        neighborCompute->useCompute();

        glGenBuffers(1, &worldChunkInfoSSBOID);
        GLuint chunkInfoIdx = 0;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunkInfoIdx, worldChunkInfoSSBOID);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO), nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

//        chunkSSBOPointer = (ChunkInfoSSBO*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        chunkSSBOPointer = (ChunkInfoSSBO*) glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER, 0, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        memset(chunkSSBOPointer, 0, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO));
//        memset(chunkSSBOPointer, 0, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO));
        if (chunkSSBOPointer == nullptr)
        {
            std::cerr << "Failed to retrieve SSBO Data." << std::endl;
            return false;
        }
        Craft::Chunk::textures = textures;
        for (int x=chunkStartX;x<chunkEndX; x++)
        {
            for (int z=chunkStartZ;z<chunkEndZ; z++)
            {
                if (chunks.find({x, z}) == chunks.end())
                {
                    chunksAdded.emplace_back(x, z);
                    int chunkIdx = ((x - minChunkCoords.x) * MAX_STORED_CHUNKS) + z - minChunkCoords.z;
                    initChunk(x, z, chunkSSBOPointer[chunkIdx].blockVisibility);
                }
            }
        }
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        calcNeighborInfo();
        if (!player.initPlayer())
        {
            return false;
        }

        sun.initSun();

        timer.resetTimer();
        userPointer = new GLFWUserPointer{};
        userPointer->camera = player.getCamera();
        userPointer->world = this;
        // Set the mouse button callback
        glfwSetWindowUserPointer(window->getWindow(), userPointer);
        glfwSetMouseButtonCallback(window->getWindow(), mouse_button_callback);
        return true;
    }
    void World::calcNeighborInfo()
    {
        // Use the compute shader program
        neighborCompute->useCompute();
        int worldWidth = (MAX_STORED_CHUNKS * CHUNK_WIDTH);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, worldChunkInfoSSBOID);
        setInt(neighborCompute->getProgram(), "worldWidth", worldWidth);
        setInt(neighborCompute->getProgram(), "numChunks", MAX_STORED_CHUNKS);
        setiVec2(neighborCompute->getProgram(), "minChunkCoords", minChunkCoords);

        // Dispatch the compute shader
        glDispatchCompute(worldWidth, CHUNK_HEIGHT, worldWidth);
        // Make sure the compute shader has finished before using the data
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
        glFinish(); // Optional: Ensure full sync (use sparingly)
        if (chunkSSBOPointer == nullptr)
        {
            std::cerr << "Failed to map buffer." << std::endl;
            return;
        }
        for (auto chunk: chunks) {
            futures.emplace_back(
                    pool.enqueue([this, chunk]() {
                        int idx = ((chunk.first.x - minChunkCoords.x) * MAX_STORED_CHUNKS) + chunk.first.z - minChunkCoords.z;
                        chunks[chunk.first]->initElementBuffer(chunkSSBOPointer[idx].blockVisibility);
                    })
            );
        }
        chunksAdded.clear();
        for (auto& future: futures) {
            future.get();
        }
        futures.clear();

        // Ensure CPU writes are visible to the GPU before issuing more GPU commands
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    void World::translateChunkSSBOData(Coordinate2D<int> directionDiff)
    {
        neighborCompute->useCompute();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, worldChunkInfoSSBOID);
        if (chunkSSBOPointer == nullptr)
        {
            std::cerr << "Failed to retrieve SSBO Data." << std::endl;
            return;
        }
        if (directionDiff.x == 1)
        {
            for (int i=0; i<TOTAL_MAX_CHUNKS; i+=MAX_STORED_CHUNKS)
            {
                for (int j=0;j<MAX_STORED_CHUNKS;j++)
                {
                    if (i >= TOTAL_MAX_CHUNKS - MAX_STORED_CHUNKS)
                    {
                        // Reset top
                        memset(&chunkSSBOPointer[i + j], 0, sizeof(ChunkInfoSSBO));
                        continue;
                    }
                    chunkSSBOPointer[i + j] = chunkSSBOPointer[i + MAX_STORED_CHUNKS + j];
                }
            }
        }
        else if (directionDiff.x == -1)
        {
            for (int i=TOTAL_MAX_CHUNKS-1; i>-1; i-=MAX_STORED_CHUNKS)
            {
                for (int j=0;j<MAX_STORED_CHUNKS;j++)
                {
                    if (i < MAX_STORED_CHUNKS)
                    {
                        // Reset top
                        memset(&chunkSSBOPointer[i - j], 0, sizeof(ChunkInfoSSBO));
                        continue;
                    }
                    chunkSSBOPointer[i - j] = chunkSSBOPointer[i - MAX_STORED_CHUNKS - j];
                }
            }
        }
        else if (directionDiff.z == 1)
        {
            for (int i=0; i<MAX_STORED_CHUNKS; i++)
            {
                for (int j=0;j<TOTAL_MAX_CHUNKS;j+=MAX_STORED_CHUNKS)
                {
                    if (i % MAX_STORED_CHUNKS == MAX_STORED_CHUNKS - 1 )
                    {
                        // Reset top
                        memset(&chunkSSBOPointer[i + j], 0, sizeof(ChunkInfoSSBO));
                        continue;
                    }
                    chunkSSBOPointer[i + j] = chunkSSBOPointer[i + 1 + j];
                }
            }
        }
        else if (directionDiff.z == -1)
        {
            for (int i=MAX_STORED_CHUNKS-1; i>-1; i--)
            {
                for (int j=0;j<TOTAL_MAX_CHUNKS;j+=MAX_STORED_CHUNKS)
                {
                    if (i % MAX_STORED_CHUNKS == 0)
                    {
                        // Reset top
                        memset(&chunkSSBOPointer[i + j], 0, sizeof(ChunkInfoSSBO));
                        continue;
                    }
                    chunkSSBOPointer[i + j] = chunkSSBOPointer[i - 1 + j];
                }
            }
        }
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    }
    void World::updateChunksLoaded(Coordinate2D<int> directionDiff) {
        updateChunkBounds();

        Engine::Timer timer{};
        timer.startStopWatch();
        float currTime = timer.lapStopWatch(), prevTime;

        translateChunkSSBOData(directionDiff);
        prevTime = currTime;
        currTime = timer.lapStopWatch();
        std::cout << "Time for Translating chunk neighbor data: " << (currTime - prevTime) << " ms " << std::endl;

        neighborCompute->useCompute();
        futures.emplace_back(
            pool.enqueue([this]() {
                if (chunkSSBOPointer == nullptr)
                {
                    std::cerr << "Failed to retrieve SSBO Data." << std::endl;
                    return;
                }
                for (int x = chunkStartX; x < chunkEndX; x++)
                {
                    for (int z = chunkStartZ; z < chunkEndZ; z++)
                    {
                        // Create new Chunks
                        Coordinate2D<int> currChunkCoord{x, z};
                        if (chunks.find(currChunkCoord) == chunks.end())
                        {
                            chunksAdded.push_back(currChunkCoord);
                            int idx = ((currChunkCoord.x - minChunkCoords.x) * MAX_STORED_CHUNKS) + currChunkCoord.z - minChunkCoords.z;
                            initChunk((int) currChunkCoord.x, (int) currChunkCoord.z, chunkSSBOPointer[idx].blockVisibility);
                        }
                    }
                }

                needsToUpdateNeighbors = true;
            })
        );

        for (auto chunkIter = chunks.begin(); chunkIter != chunks.end(); )
        {
            if (
                    chunkIter->first.x < chunkStartX || chunkIter->first.x >= chunkEndX ||
                    chunkIter->first.z < chunkStartZ || chunkIter->first.z >= chunkEndZ
                    )
            {
                delete chunkIter->second;
                chunkIter = chunks.erase(chunkIter);
            }
            else
            {
                chunkIter++;
            }
        }



//        prevTime = currTime;
//        currTime = timer.lapStopWatch();
//        std::cout << "Time for init Chunks: " << (currTime - prevTime) << " ms " << std::endl;
//
//        prevTime = currTime;
//        currTime = timer.lapStopWatch();
//        std::cout << "Time for Delete old Chunks: " << (currTime - prevTime) << " ms " << std::endl;

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
//        calcNeighborInfo(chunksAdded);
//        prevTime = currTime;
//        currTime = timer.lapStopWatch();
//        std::cout << "Time for calc neighbors: " << (currTime - prevTime) << " ms " << std::endl;
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