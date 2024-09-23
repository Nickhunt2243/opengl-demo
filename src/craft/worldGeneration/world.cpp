
#include "world.hpp"
#include "../misc/globals.hpp"

namespace Craft
{
    World::World(
            Engine::Window* window,
            Engine::Program* blockProgram,
            Engine::Program* worldProgram,
            Engine::Compute* neighborCompute,
            Engine::Compute* ambientOccCompute,
            uint32_t width,
            uint32_t height
    )
            : coords{}
            , window{window}
            , blockProgram{blockProgram}
            , worldProgram{worldProgram}
            , neighborCompute{neighborCompute}
            , ambientOccCompute{ambientOccCompute}
            , timer()
            , player{&timer, window, blockProgram, worldProgram, width, height, &coords, &coordsMutex}
            , sun{worldProgram}
    {
        updateChunkBounds();
    }
    World::~World()
    {
        if (blockSSBOPointer)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockSSBO);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
        if (chunkSSBOPointer)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkSSBO);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
        if (idxSSBOPointer)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, idxSSBO);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
        if (drawCommandBufferPointer)
        {
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBO);
            glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
        }

        delete textures;
        delete userPointer;
        if (blockSSBO != 0)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockSSBO);
            glDeleteVertexArrays(1, &(blockSSBO));
        }
        if (chunkSSBO != 0)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkSSBO);
            glDeleteVertexArrays(1, &(chunkSSBO));
        }
        if (idxSSBO != 0)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, idxSSBO);
            glDeleteVertexArrays(1, &(idxSSBO));
        }

        if (VBO != 0)
        {
            glDeleteVertexArrays(1, &(VBO));
        }
        if (InstanceVBO != 0)
        {
            glDeleteVertexArrays(1, &(InstanceVBO));
        }
        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &(VAO));
        }
        delete[] instanceCount;
    }
    BlockInfo calcBlockData(Coordinate<int> blockData)
    {
        Coordinate2D<int> chunkPos{(int) floor((float) blockData.x / 16.0f), (int) floor((float) blockData.z / 16.0f)};
        Coordinate<int> blockRelPos{blockData.x - (chunkPos.x * 16), blockData.y, blockData.z - (chunkPos.z * 16)};

        return {blockRelPos, chunkPos};
    }
    void World::updateNeighbors(BlockInfo info)
    {
        // Construct neighbor info:
        std::vector<Coordinate<int>> blockOffsets = {
                {0, -1, 0}, {0, 1, 0},  {-1, 0, 0},
                {1, 0, 0},  {0, 0, -1}, {0, 0, 1}
        };
        for (int side=0; side<SIDES_PER_BLOCK; side++)
        {
            BlockInfo otherBlock = getBlockInfo(info.block + blockOffsets[side], info.chunk);
            int chunkIdx = chunks[otherBlock.chunk]->chunkIdx;
            int chunkOffset = chunkIdx * BLOCKS_IN_CHUNK;
            int blockIdx = chunkOffset + (otherBlock.block.y * CHUNK_SIZE) + (otherBlock.block.z * CHUNK_WIDTH) + otherBlock.block.x;
            int sideOffset = side * TOTAL_MAX_CHUNKS;
            NeighborInfo neighborInfo = blockSSBOPointer[blockIdx];
            if ((neighborInfo.sideData & 1) == 1)
            {
                int sideIdx = sideOffset + chunkIdx;
                idxSSBOPointer[(side * BLOCKS_IN_WORLD) + chunkOffset + instanceCount[sideIdx]] = blockIdx;
                instanceCount[sideIdx] += 1;
                drawCommandBufferPointer[sideIdx].instanceCount = instanceCount[sideIdx];
            }
        }
    }
    void World::updateNeighborsCreatedBlock(BlockInfo info)
    {
        // Construct neighbor info:
        for (int side=0; side<SIDES_PER_BLOCK; side++)
        {
            int chunkIdx = chunks[info.chunk]->chunkIdx;
            int chunkOffset = chunkIdx * BLOCKS_IN_CHUNK;
            int blockIdx = chunkOffset + (info.block.y * CHUNK_SIZE) + (info.block.z * CHUNK_WIDTH) + info.block.x;
            int sideOffset = side * TOTAL_MAX_CHUNKS;
            NeighborInfo neighborInfo = blockSSBOPointer[blockIdx];
            if ((neighborInfo.sideData & 1) == 1)
            {
                int sideIdx = sideOffset + chunkIdx;
                idxSSBOPointer[(side * BLOCKS_IN_WORLD) + chunkOffset + instanceCount[sideIdx]] = blockIdx;
                instanceCount[sideIdx] += 1;
                drawCommandBufferPointer[sideIdx].instanceCount = instanceCount[sideIdx];
            }
        }
    }
    void World::appendAdditionalAffectedChunks(BlockInfo info)
    {
        if (info.block.x == 0)
        {
            Coordinate2D<int> newChunk {info.chunk.x-1, info.chunk.z};
            chunksToUpdateAmbientInfo.push_back(newChunk);
        }
        else if (info.block.x == 15)
        {
            Coordinate2D<int> newChunk {info.chunk.x+1, info.chunk.z};
            chunksToUpdateAmbientInfo.push_back(newChunk);
        }
        if (info.block.z == 0)
        {
            Coordinate2D<int> newChunk {info.chunk.x, info.chunk.z-1};
            chunksToUpdateAmbientInfo.push_back(newChunk);
        }
        else if (info.block.z == 15)
        {
            Coordinate2D<int> newChunk {info.chunk.x, info.chunk.z+1};
            chunksToUpdateAmbientInfo.push_back(newChunk);
        }
    }
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        auto userPointerData = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(window));
        World* world = userPointerData->world;
        if (world == nullptr) return;
        std::unordered_set<Coordinate2D<int>> chunksToUpdate{};
        std::cout << "Mouse clicked." << std::endl;
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            // Handle left mouse button press
            if (world->player.lookAtBlock != nullptr)
            {
                Coordinate<int> blockToDelete = *world->player.lookAtBlock;
                BlockInfo info = calcBlockData(blockToDelete);
                auto chunkIter = world->chunks.find(info.chunk);
                if (chunkIter != world->chunks.end())
                {
                    world->chunksToUpdateAmbientInfo.push_back(info.chunk);
                    world->appendAdditionalAffectedChunks(info);
                    world->calcAmbientOcclusionInfo();
                    world->updateNeighbors(info);
                    chunkIter->second->deleteBlock(info.block, world->blockSSBOPointer);
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
                auto chunkIter = world->chunks.find(info.chunk);
                if (chunkIter != world->chunks.end())
                {
                    chunkIter->second->createBlock(info.block, world->textures, world->blockSSBOPointer);
                    world->chunksToUpdateAmbientInfo.push_back(info.chunk);
                    world->appendAdditionalAffectedChunks(info);
                    world->calcAmbientOcclusionInfo();
                    world->updateNeighborsCreatedBlock(info);
                }
            }
        }
        else if (action == GLFW_RELEASE)
        {
            std::cout << "Mouse button released." << std::endl;
            // Handle mouse button release
        }
    }
    void World::initBuffers()
    {
        // Init VAO, SSBO, and VBOs
        instanceCount = new int[SIDES_PER_BLOCK * TOTAL_MAX_CHUNKS];
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &InstanceVBO);
        glGenBuffers(1, &indirectBO);
        glGenBuffers(1, &chunkSSBO);
        glGenBuffers(1, &blockSSBO);
        glGenBuffers(1, &idxSSBO);
        // Bind VAO
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,  (GLsizei) (blockData.size() * sizeof(int)), blockData.data(), GL_STATIC_DRAW);
        auto stride = (GLsizei) ((blockData.size() / VERTICES_PER_BLOCK) * sizeof(int));
        glVertexAttribIPointer(0, 3, GL_INT, stride, nullptr);
        glVertexAttribIPointer(1, 3, GL_INT, stride, (void*) (3 * sizeof(int)));
        glVertexAttribIPointer(2, 2, GL_INT, stride, (void*) (6 * sizeof(int)));
        glVertexAttribIPointer(3, 1, GL_INT, stride, (void*) (8 * sizeof(int)));
        glVertexAttribIPointer(4, 1, GL_INT, stride, (void*) (9 * sizeof(int)));
        glVertexAttribIPointer(5, 2, GL_INT, stride, (void*) (10 * sizeof(int)));
        for (int attribPointer=0; attribPointer<6;attribPointer++)
        {
            glEnableVertexAttribArray(attribPointer);
            glVertexAttribDivisor(attribPointer, 0);
        }

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBO);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, SIDES_PER_BLOCK * TOTAL_MAX_CHUNKS * sizeof(DrawArraysIndirectCommand), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        drawCommandBufferPointer = (DrawArraysIndirectCommand*) glMapBufferRange(
                GL_DRAW_INDIRECT_BUFFER, 0, SIDES_PER_BLOCK * TOTAL_MAX_CHUNKS * sizeof(DrawArraysIndirectCommand),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );

    // ChunkSSBO Binding and initialization
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkSSBO);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, TOTAL_MAX_CHUNKS * sizeof(Coordinate2D<int>), nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        chunkSSBOPointer = (Coordinate2D<int>*) glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER, 0, TOTAL_MAX_CHUNKS * sizeof(Coordinate2D<int>),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        memset(chunkSSBOPointer, 0, TOTAL_MAX_CHUNKS * sizeof(Coordinate2D<int>));
    // blockSSBO Binding and initialization
        // Bind and initialize the storage for the SSBO.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockSSBO);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, BLOCKS_IN_WORLD * sizeof(NeighborInfo), nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        blockSSBOPointer = (NeighborInfo*) glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER, 0, BLOCKS_IN_WORLD * sizeof(NeighborInfo),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        memset(blockSSBOPointer, 0, BLOCKS_IN_WORLD * sizeof(NeighborInfo));
    // idxSSBO Binding and initialization
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, idxSSBO);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, 6 * BLOCKS_IN_WORLD * sizeof(int), nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        idxSSBOPointer = (int*) glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER, 0, 6 * BLOCKS_IN_WORLD * sizeof(int),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        memset(idxSSBOPointer, 0, 6 * BLOCKS_IN_WORLD * sizeof(int));

        int blockInfoIdx = 0;
        int chunkInfoIdx = 1;
        int idxInfoIdx = 2;
        // Define the SSBO for the neighbor compute shader.
        neighborCompute->useCompute();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, blockInfoIdx, blockSSBO);

        // Define the SSBO for the block shaders.
        blockProgram->useProgram();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, blockInfoIdx, blockSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunkInfoIdx, chunkSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, idxInfoIdx, idxSSBO);

        // Define the SSBO for the ambient occlusion compute shader.
        ambientOccCompute->useCompute();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, blockInfoIdx, blockSSBO);
    }
    void World::initChunk(Coordinate2D<int> chunkPos)
    {
        {
            std::lock_guard<std::mutex> lock(chunkMutex);
            chunks.emplace(chunkPos, std::make_unique<Chunk>(chunkPos, &coords, &coordsMutex));
        }
        chunks[chunkPos]->initChunk(blockSSBOPointer, textures);
        chunkSSBOPointer[chunks[chunkPos]->chunkIdx] = chunkPos;
    }
    bool World::initWorld()
    {
        // init world
        textures = new Craft::Textures();
        textures->initTextures(blockProgram->getProgram());

        initBuffers();
        if (
                blockSSBOPointer == nullptr ||
                chunkSSBOPointer == nullptr ||
                idxSSBOPointer == nullptr ||
                drawCommandBufferPointer == nullptr
            )
        {
            std::cerr << "Failed to retrieve SSBO Data." << std::endl;
            return false;
        }

        for (int x=chunkStartX; x<chunkEndX; x++)
        {
            futures.emplace_back(
                pool.enqueue([this, x]()
                {
                    for (int z=chunkStartZ; z<chunkEndZ; z++)
                    {
                        Coordinate2D<int> chunkPos{x, z};
                        if (chunks.find(chunkPos) == chunks.end())
                        {
                            initChunk(chunkPos);
                            {
                                std::lock_guard<std::mutex> lock(chunkNeighborMutex);
                                chunksToUpdateNeighborInfo.push_back(chunkPos);
                            }
                            {
                                std::lock_guard<std::mutex> lock(chunkAmbientMutex);
                                chunksToUpdateAmbientInfo.push_back(chunkPos);
                            }
                        }
                    }
                })
            );
        }
        for (auto& future: futures)
        {
            future.get();
        }
        futures.clear();

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
        calcNeighborInfo();
        calcAmbientOcclusionInfo();
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
        neighborCompute->useCompute();
        setInt(neighborCompute->getProgram(), "u_numChunks", TOTAL_CHUNK_WIDTH);
        setInt(neighborCompute->getProgram(), "u_renderDistance", RENDER_DISTANCE);
        // Dispatch the compute shader
        {
            std::lock_guard<std::mutex> lock(chunkNeighborMutex);
            for (const auto& chunkIter: chunksToUpdateNeighborInfo)
            {
                setiVec2(neighborCompute->getProgram(), "u_chunkPos", chunkIter);
                glDispatchCompute(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH);
                chunksToUpdateVBOInfo.push_back(chunkIter);
            }

            chunksToUpdateNeighborInfo.clear();
        }
        // Make sure the compute shader has finished before using the data
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
        glFinish();
        updateInstanceIdxVBO();
    }
    void World::calcAmbientOcclusionInfo()
    {
        ambientOccCompute->useCompute();
        setInt(ambientOccCompute->getProgram(), "u_numChunks", TOTAL_CHUNK_WIDTH);
        setInt(ambientOccCompute->getProgram(), "u_renderDistance", RENDER_DISTANCE);
        {
            std::lock_guard<std::mutex> lock(chunkAmbientMutex);
            for (const auto& chunkIter: chunksToUpdateAmbientInfo)
            {
                setiVec2(ambientOccCompute->getProgram(), "u_chunkPos", chunkIter);
                glDispatchCompute(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH);
            }
            chunksToUpdateAmbientInfo.clear();
        }
        // Make sure the compute shader has finished before using the data
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    void World::updateInstanceIdxVBO()
    {
        std::lock_guard<std::mutex> lock(chunkMutex);
        if (instanceCount == nullptr) return;
        for (int side=0; side<SIDES_PER_BLOCK; side++)
        {
            for (auto& chunk: chunksToUpdateVBOInfo)
            {
                int sideIdx = (side * TOTAL_MAX_CHUNKS) + chunks[chunk]->chunkIdx;
                instanceCount[sideIdx] = 0;
            }
        }
        {
            std::lock_guard<std::mutex> lock(chunkVBOMutex);
            for (auto& chunkCoord: chunksToUpdateVBOInfo)
            {
//                for (int side=0; side<SIDES_PER_BLOCK; side++)
//                {
                    Chunk* chunk = chunks[chunkCoord].get();
                    int chunkIdx = chunk->chunkIdx;
                    int chunkOffset = chunk->chunkIdx * BLOCKS_IN_CHUNK;
                    futures.emplace_back(
                        pool.enqueue([this, chunk, chunkOffset, chunkIdx]() {

                            std::lock_guard<std::mutex> lock(chunkMutex);
                            for (int side=0; side<SIDES_PER_BLOCK; side++)
                            {
                                int sideOffset = side * TOTAL_MAX_CHUNKS;
                                int blockIdx;
                                for (auto &block: chunk->blocksMap)
                                {
                                    blockIdx = chunkOffset + (block.first.y * CHUNK_SIZE) +
                                               (block.first.z * CHUNK_WIDTH) + block.first.x;
                                    NeighborInfo info = blockSSBOPointer[blockIdx];
                                    if ((info.sideData & 1) == 1) {
                                        int sideShift = side + 1;
                                        if (((info.sideData >> sideShift) & 1) == 1) {
                                            int sideIdx = sideOffset + chunkIdx;
                                            idxSSBOPointer[(side * BLOCKS_IN_WORLD) + chunkOffset +
                                                           instanceCount[sideIdx]] = blockIdx;
                                            instanceCount[sideIdx] += 1;
                                        }
                                    }
                                }
                            }

                            for (int side=0; side<SIDES_PER_BLOCK; side++)
                            {
                                int sideOffset = side * TOTAL_MAX_CHUNKS;
                                // Update Draw Commands:
                                DrawArraysIndirectCommand currCommand{};
                                currCommand.first = side * VERTICES_PER_SIDE;
                                currCommand.count = VERTICES_PER_SIDE;
                                currCommand.instanceCount = instanceCount[sideOffset + chunkIdx];
                                currCommand.baseInstance = (side * BLOCKS_IN_WORLD) + chunkOffset;
                                drawCommandBufferPointer[sideOffset + chunkIdx] = currCommand;
                            }
                        })
                    );
//                }
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
            chunksToUpdateVBOInfo.clear();
        }
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    void World::updateChunkBounds()
    {
        chunkStartX = (int) player.originChunk.x - (RENDER_DISTANCE);
        chunkStartZ = (int) player.originChunk.z - (RENDER_DISTANCE);
        chunkEndX = (int) player.originChunk.x + (RENDER_DISTANCE) + 1;
        chunkEndZ = (int) player.originChunk.z + (RENDER_DISTANCE) + 1;
    }
    void World::updateChunksLoaded(Coordinate2D<int> directionDiff)
    {
        updateChunkBounds();

        futures.emplace_back(
            pool.enqueue([this, directionDiff]()
            {
                {
                    std::lock_guard<std::mutex> lock(chunkMutex);
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
                }
                for (int x=chunkStartX; x<chunkEndX; x++)
                {
                    for (int z=chunkStartZ; z<chunkEndZ; z++)
                    {
                        // Create new Chunks
                        Coordinate2D<int> chunkPos{x, z};
                        if (chunks.find(chunkPos) == chunks.end())
                        {
                            int chunkIdx = ((findChunkIdx(x) * TOTAL_CHUNK_WIDTH) + findChunkIdx(z));
                            int chunkOffset = chunkIdx * BLOCKS_IN_CHUNK;
                            memset(blockSSBOPointer + chunkOffset, 0, BLOCKS_IN_CHUNK * sizeof(NeighborInfo));
                            for (int side = 0; side < SIDES_PER_BLOCK; side++) {
                                int idx = (side * TOTAL_MAX_CHUNKS) + chunkIdx;
                                drawCommandBufferPointer[idx].instanceCount = 0;
                            }
                            initChunk(chunkPos);
                            {
                                std::lock_guard<std::mutex> lock(chunkNeighborMutex);
                                chunksToUpdateNeighborInfo.push_back(chunkPos);
                                chunksToUpdateNeighborInfo.push_back(chunkPos - directionDiff);
                            }
                            {
                                std::lock_guard<std::mutex> lock(chunkAmbientMutex);
                                chunksToUpdateAmbientInfo.push_back(chunkPos);
                                chunksToUpdateAmbientInfo.push_back(chunkPos - directionDiff);
                            }
                        }
                    }
                }
            })
        );

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
        if (chunksToUpdateNeighborInfo.size() >= (2 * TOTAL_CHUNK_WIDTH))
        {
            calcNeighborInfo();
        }
        if (chunksToUpdateAmbientInfo.size() >= (2 * TOTAL_CHUNK_WIDTH))
        {
            calcAmbientOcclusionInfo();
        }
        return true;
    }

    bool World::drawWorld() {
        blockProgram->useProgram();
        timer.incFrames();

        float x = ((float) sun.getTime().hours * M_PI / 12) + M_PI;
        float cosX = cos(x);
        float newLightLevel = (7 * cosX + 5) + 4 * abs(cosX);
        setFloat(blockProgram->getProgram(), "u_defaultLightLevel", newLightLevel);
        glBindVertexArray(VAO);
        glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, SIDES_PER_BLOCK * TOTAL_MAX_CHUNKS, 0);
        sun.drawLight((float) player.getWorldX(), (float) player.entityY, (float) player.getWorldZ());
//        std::cout << "FPS: " << timer.getFPS() << std::endl;
        return true;
    }
}
