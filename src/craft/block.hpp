//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_BLOCK_HPP
#define OPENGLDEMO_BLOCK_HPP
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "types.hpp"
#include "../setup/program.hpp"
#include "coordinate.hpp"

namespace Craft
{
    class Block
    {
    public:
        Block(
                Coordinate& coord,
                std::string& blockType,
                GLuint program,
                const blockTexture& textures
        );
        ~Block() = default;
        void defineSides();
        bool draw();
        void setHasNeighbors(const HasNeighbors& neighbors);
    private:
        HasNeighbors hasNeighbors{};
        std::string blockType;
        std::string textureLocation;
        GLuint program;
        blockTexture textures;
        Coordinate position;

        void sendNeighborData() const;
        void activateTextures() const;
    };
}

#endif //OPENGLDEMO_BLOCK_HPP
