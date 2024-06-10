//
// Created by admin on 5/27/2024.
//

#ifndef OPENGLDEMO_TEXTURES_HPP
#define OPENGLDEMO_TEXTURES_HPP

#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "types.hpp"
#include "../helpers/helpers.hpp"

namespace Craft
{
    class Textures
    {
    public:
        Textures() = default;
        ~Textures();
        /**
         * Initialize the textures of the program.
         *
         * Textures are defined through the assets/json/texture_mapping.json file.
         * This file allows us to easily manage which files go to which block types and side combinations.
         *
         * @param program: The program to attach the textures to.
         * @return:        True if the textures were initialize, else False.
         */
        bool initTextures(GLuint program);
        /**
         * Retrieve the texture struct of a given block type.
         *
         * @param type: The enum type of the block.
         * @return:     A blockTexture struct.
         */
        [[nodiscard]] blockTexture getTexture(BlockType type) const;
    private:
        /**
         * Initialize the image data into a Sampler2DArray object where each layer is a different texture.
         *
         * @param imageData: A mapping of file paths to ImageLoadResult structs
         * @return:          The GLuint of the 2D array texture.
         *
         * @see ImageLoadResult
         */
        static GLuint initTextureFromData(std::unordered_map<std::string, ImageLoadResult>& imageData);
        /// A mapping of BlockType enum to a struct containing the block's texture information.
        std::unordered_map<BlockType, blockTexture> textureMapping{};
    };
}
#endif //OPENGLDEMO_TEXTURES_HPP
