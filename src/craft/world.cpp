//
// Created by admin on 5/26/2024.
//

#include "world.hpp"

namespace Craft
{
    bool World::initializeWorld(GLuint VAO, GLuint VBO)
    {
        block = new Block(0.0, 0.0, 0.0, "grass", VAO, VBO);
    }

    bool World::drawWorld()
    {
        return block->draw();
    }
}