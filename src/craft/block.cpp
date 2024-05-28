//
// Created by admin on 5/26/2024.
//

#include "block.hpp"
#include <iostream>

#include "../helpers/helpers.hpp"
#include "textures.hpp"

namespace Craft
{

    Block::Block(
            Coordinate& coord,
            std::string& blockType,
            GLuint program,
            const blockTexture& textures
    )
        : position( coord )
        , blockType( blockType )
        , program(program)
        , textures(textures)
    {}
    void defineSides() {

    }
    void Block::activateTextures() const
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures.top->id);
        Helpers::setVec4(program, "u_textureColorMapping[0]", textures.top->colorMapping);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures.bottom->id);
        Helpers::setVec4(program, "u_textureColorMapping[1]", textures.bottom->colorMapping);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures.front->id);
        Helpers::setVec4(program, "u_textureColorMapping[2]", textures.front->colorMapping);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures.right->id);
        Helpers::setVec4(program, "u_textureColorMapping[3]", textures.right->colorMapping);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textures.back->id);
        Helpers::setVec4(program, "u_textureColorMapping[4]", textures.back->colorMapping);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, textures.left->id);
        Helpers::setVec4(program, "u_textureColorMapping[5]", textures.left->colorMapping);
    }
    void Block::setHasNeighbors(const HasNeighbors& neighbors) {
        hasNeighbors = neighbors;
    }
    void Block::sendNeighborData() const
    {
        Helpers::setBool(program, "drawFace[0]", !hasNeighbors.top);
        Helpers::setBool(program, "drawFace[1]", !hasNeighbors.bottom);
        Helpers::setBool(program, "drawFace[2]", !hasNeighbors.front);
        Helpers::setBool(program, "drawFace[3]", !hasNeighbors.right);
        Helpers::setBool(program, "drawFace[4]", !hasNeighbors.back);
        Helpers::setBool(program, "drawFace[5]", !hasNeighbors.left);
    }
    bool Block::draw()
    {
        // Because we are currently only using one VAO and one VBO, there is no need to bind and unbind each draw call.
        sendNeighborData();
        activateTextures();
        glBufferData(
                GL_ARRAY_BUFFER,
                (GLsizeiptr) (3 * sizeof(float)),
                (float*) position,
                GL_STATIC_DRAW
            );
        glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                3 * sizeof(float),
                nullptr
            );
        glDrawArrays(GL_POINTS, 0, 1);
        return true;
    }
}