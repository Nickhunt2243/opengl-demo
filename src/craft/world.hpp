//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_WORLD_HPP
#define OPENGLDEMO_WORLD_HPP

#include <unordered_map>
#include <functional>

#include "block.hpp"
#include "textures.hpp"
#include "coordinate.hpp"

namespace Craft
{
    class World
    {
    public:
        World() = default;
        ~World() = default;

        void initWorld(GLuint program);
        bool drawWorld();
        void findNeighbors();
    private:
        Textures textures{};
        std::unordered_map<Coordinate, Block> blocks{};

        void initBlock( std::string blockType, GLuint program, float x, float y, float z);
    };
}

#endif //OPENGLDEMO_WORLD_HPP
