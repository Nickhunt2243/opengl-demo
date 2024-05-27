//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_WORLD_HPP
#define OPENGLDEMO_WORLD_HPP

#include "block.hpp"

namespace Craft
{
    class World
    {
    public:
        World() = default;
        ~World() = default;

        bool initializeWorld(GLuint VAO, GLuint VBO);
        bool drawWorld();
    private:
        Block* block;
    };
}

#endif //OPENGLDEMO_WORLD_HPP
