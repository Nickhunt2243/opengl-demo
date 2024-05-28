//
// Created by admin on 5/26/2024.
//
#include <iostream>

#include <string>
#include <unordered_set>
#include <sstream>
#include <thread>
#include <mutex>

#include "world.hpp"
#include "../helpers/helpers.hpp"

namespace Craft
{
    static std::mutex mutex{};
    void World::initBlock( std::string blockType, GLuint program, float x, float y, float z) {
        blockTexture texture = textures.getTexture(blockType);
        Coordinate coord{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
        Block block{coord, blockType, program, texture};
        // Ensure the blocks map is accessed in a thread-safe manner
        std::lock_guard<std::mutex> lock(mutex);
        blocks.insert({coord, block});
    }
    void World::initWorld(GLuint program)
    {
        textures.initTextures();
        std::cout << "Creating blocks" << std::endl;
        std::string blockType = "grass";
        std::ostringstream positionString;
        std::vector<std::thread> threads;
        for (int y=-1;y>-17;y--) {
            for (int z = -8; z < 9; z++) {
                for (int x = -8; x < 9; x++) {
                    threads.emplace_back(&World::initBlock, this, blockType, program, x, y, z);
                }
            }
            if (y < -2) {
                blockType = "stone";
            } else if (y<-0) {
                blockType = "dirt";
            }
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    void World::findNeighbors() {
        std::vector<std::thread> threads;
        for (const auto& iter: blocks) {
            Coordinate currCoord = iter.first;
//            Block currBlock = iter.second;
            threads.emplace_back([this, currCoord]() {
                HasNeighbors hasNeighbors{};
                // top
                if (blocks.find(currCoord.add(0.0f,1.0f,0.0f)) != blocks.end()) {
                    hasNeighbors.top = true;
                }
                // bottom
                if (blocks.find(currCoord.add(0.0f,-1.0f,0.0f)) != blocks.end()) {
                    hasNeighbors.bottom = true;
                }
                // front
                if (blocks.find(currCoord.add(0.0f,0.0f,1.0f)) != blocks.end()) {
                    hasNeighbors.front = true;
                }
                // right
                if (blocks.find(currCoord.add(1.0f,0.0f,0.0f)) != blocks.end()) {
                    hasNeighbors.right = true;
                }
                // back
                if (blocks.find(currCoord.add(0.0f,0.0f,-1.0f)) != blocks.end()) {
                    hasNeighbors.back = true;
                }
                // left
                if (blocks.find(currCoord.add(-1.0f,0.0f,0.0f)) != blocks.end()) {
                    hasNeighbors.left = true;
                }

                std::lock_guard<std::mutex> lock(mutex);
                blocks.at(currCoord).setHasNeighbors(hasNeighbors);
            });
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    bool World::drawWorld()
    {

        bool status = true;
        for (auto iter = blocks.begin(); iter !=blocks.end(); iter++) {
            status &= iter->second.draw();
        }
        return status;
    }
}