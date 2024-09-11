
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
            int chunkIdxX = (((info.chunk.x + RENDER_DISTANCE) % TOTAL_CHUNK_WIDTH) + TOTAL_CHUNK_WIDTH) % TOTAL_CHUNK_WIDTH;
            int chunkIdxZ = (((info.chunk.z + RENDER_DISTANCE) % TOTAL_CHUNK_WIDTH) + TOTAL_CHUNK_WIDTH) % TOTAL_CHUNK_WIDTH;

            int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
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
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->worldChunkInfoSSBOID);
                    if (world->chunkSSBOPointer == nullptr)
                    {
                        std::cerr << "Failed to retrieve SSBO Data." << std::endl;
                        return;
                    }
                    int chunkIdxX = findChunkIdx(info.chunk.x);
                    int chunkIdxZ = findChunkIdx(info.chunk.z);

                    int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
                    world->chunks[info.chunk]->deleteBlock(info.block, world->chunkSSBOPointer[chunkIdx].blockVisibility);
                    chunksToUpdate = world->updateNeighbors(info.block, info.chunk, world->chunkSSBOPointer, true);
                    for (auto chunk: chunksToUpdate)
                    {
                        world->chunks[chunk]->initBufferData(world->textures, chunkIdx);
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
                    int chunkIdxX = findChunkIdx(info.chunk.x);
                    int chunkIdxZ = findChunkIdx(info.chunk.z);
                    int chunkIdx = (chunkIdxX * TOTAL_CHUNK_WIDTH) + chunkIdxZ;
                    world->chunks[info.chunk]->createBlock(info.block, world->chunkSSBOPointer[chunkIdx].blockVisibility);
                    chunksToUpdate = world->updateNeighbors(info.block, info.chunk, world->chunkSSBOPointer, false);
                    for (auto chunk: chunksToUpdate)
                    {
                        world->chunks[chunk]->initBufferData(world->textures, chunkIdx);
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
        chunkStartX = (int) player.originChunk.x - (RENDER_DISTANCE);
        chunkStartZ = (int) player.originChunk.z - (RENDER_DISTANCE);
        chunkEndX = (int) player.originChunk.x + (RENDER_DISTANCE) + 1;
        chunkEndZ = (int) player.originChunk.z + (RENDER_DISTANCE) + 1;
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
        glDeleteBuffers(1, &worldChunkInfoSSBOID);
        if (VBO != 0 && VAO != 0)
        {
            glDeleteBuffers(1, &(VBO));
            glDeleteVertexArrays(1, &(VAO));
        }
    }
    void World::initChunk(int x, int z, NeighborInfo* visibility)
    {
        Coordinate2D<int> currChunkCoord{x, z};
        {
            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks.emplace(currChunkCoord, std::make_unique<Chunk>(blockProgram, x, z, &coords, &coordsMutex, VBO));
        }
        chunks[currChunkCoord]->initChunk(visibility);
    }
    void World::initBuffers()
    {
        // Init VAO and VBO
        if (VAO == 0)
            glGenVertexArrays(1, &VAO);
        if (VBO == 0)
            glGenBuffers(1, &VBO);

        // Bind VAO
        glBindVertexArray(VAO);
        // Bind and initialize the VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        uint32_t totalVboSize = MAX_WORLD_BLOCKS * sizeof(BlockVertexData);
        // We will map the data by range when we need to update the data.
        glBufferData(GL_ARRAY_BUFFER, totalVboSize, nullptr, GL_DYNAMIC_DRAW);
        auto stride = (GLsizei) sizeof(BlockVertexData);
        // 1 block data
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);
        // Block Pos X and Z (in world coords)
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*) (1 * sizeof(int)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*) (2 * sizeof(int)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Init SSBO Buffer
        glGenBuffers(1, &worldChunkInfoSSBOID);
        // Bind SSBO to both blockPrograms.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, worldChunkInfoSSBOID);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TOTAL_MAX_CHUNKS * sizeof(ChunkInfoSSBO), nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        GLuint chunkInfoIdx = 0;
        neighborCompute->useCompute();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunkInfoIdx, worldChunkInfoSSBOID);
        blockProgram->useProgram();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunkInfoIdx, worldChunkInfoSSBOID);
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

        blockCounts = new GLint[TOTAL_MAX_CHUNKS];
        chunkFirst = new GLsizei[TOTAL_MAX_CHUNKS];
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

                    chunks[{x, z}]->initBufferData(textures, chunkIdx);

                    GLsizei chunkVBOIdx = chunkIdx * chunkOffset;
                    blockCounts[chunkIdx] = chunks[{x, z}]->vboSize;
                    chunkFirst[chunkIdx] = (GLsizei) (chunkVBOIdx);
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, worldChunkInfoSSBOID);
        setInt(neighborCompute->getProgram(), "u_WorldWidth", worldWidth);
        setInt(neighborCompute->getProgram(), "u_NumChunks", TOTAL_CHUNK_WIDTH);
        setInt(neighborCompute->getProgram(), "u_RenderDistance", RENDER_DISTANCE);
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
                blockCounts[chunkIdx] = 0;
            }
        }
        else if (directionDiff.z != 0)
        {
            int z = findChunkIdx(newChunkCoord.z);
            for (int x=0; x<TOTAL_CHUNK_WIDTH; x++)
            {
                int chunkIdx = (x * TOTAL_CHUNK_WIDTH) + z;
                memset(&chunkSSBOPointer[chunkIdx], 0, sizeof(ChunkInfoSSBO));
                blockCounts[chunkIdx] = 0;
            }
        }
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    void World::updateChunksLoaded(Coordinate2D<int> directionDiff)
    {
        updateChunkBounds();
        translateChunkSSBOData(directionDiff);

//        futures.emplace_back(
//            pool.enqueue([this]() {
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

                            chunks[currChunkCoord]->initBufferData(textures, chunkIdx);

                            blockCounts[chunkIdx] = chunks[currChunkCoord]->vboSize;
                        }
                    }
                }
                needsToUpdateNeighbors = true;
//            })
//        );

        for (auto chunkIter = chunks.begin(); chunkIter != chunks.end(); )
        {
            if (
                    chunkIter->first.x < chunkStartX || chunkIter->first.x >= chunkEndX ||
                    chunkIter->first.z < chunkStartZ || chunkIter->first.z >= chunkEndZ
                )
            {
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

        setFloat(blockProgram->getProgram(), "u_DefaultLightLevel", newLightLevelEq1 + newLightLevelEq2);
        setInt(blockProgram->getProgram(), "u_NumChunks", TOTAL_CHUNK_WIDTH);
        setInt(blockProgram->getProgram(), "u_RenderDistance", RENDER_DISTANCE);

        glMultiDrawArrays(GL_POINTS, chunkFirst, blockCounts, TOTAL_MAX_CHUNKS);
        glBindVertexArray(0);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
        glFinish();
        sun.drawLight((float) player.getWorldX(), (float) player.entityY, (float) player.getWorldZ());
        std::cout << "FPS: " << timer.getFPS() << std::endl;

        return true;
    }
}