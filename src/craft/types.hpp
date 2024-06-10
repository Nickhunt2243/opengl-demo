//
// Created by admin on 5/27/2024.
//

#ifndef OPENGLDEMO_TYPES_HPP
#define OPENGLDEMO_TYPES_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <stb/stb_image.h>

//#include "block.hpp"

namespace Craft
{
    struct textureData {
        /// The layer of the texture in the sampler2DArray
        GLuint layer;
        /// The color mapping of the texture.
        glm::vec4 colorMapping;

        ~textureData() {
            glDeleteTextures(1, &layer);
        }
    };

    /// A struct containing all the information of the block's textures.
    struct blockTexture {
        textureData *top;
        textureData *bottom;
        textureData *front;
        textureData *right;
        textureData *back;
        textureData *left;
    };

    /**
     * A struct containing information on a block's neighbors.
     *
     * 0: Has neighbor, do NOT draw the side.
     * 1: No neighbor, draw the side.
     *
     */
    struct NeighborsInfo {
        uint8_t top: 1;
        uint8_t bottom: 1;
        uint8_t front: 1;
        uint8_t right: 1;
        uint8_t back: 1;
        uint8_t left: 1;

        /**
         * A function that determines the number of sides without neighbors (how many sides to draw).
         * @return: The number of sides without neighbors.
         */
        [[nodiscard]] int sum() const {
            return top + bottom + front + right + back + left;
        }
    };

    /**
     * A enum class of types of block's.
     */
    enum class BlockType {
        GRASS,
        DIRT,
        STONE
    };

    /**
     * The << operator for the enum class BlockType for troubleshooting.
     *
     * @param os:        The ostream to append the text to.
     * @param blockType: The enum type of the block.
     * @return:          The updated ostream.
     */
    extern std::ostream &operator<<(std::ostream &os, BlockType blockType);

    /// A mapping of lowercase block names to their type in the enum class BlockType
    extern std::unordered_map<std::string, BlockType> stringToBlockType;
}

#endif //OPENGLDEMO_TYPES_HPP
