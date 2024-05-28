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

namespace Craft
{
    class Textures
    {
    public:
        Textures() = default;
        ~Textures();

        bool initTextures();
        blockTexture getTexture(std::string& name);

    private:
        GLuint initTextureFromData(Helpers::ImageData* imageData);
        std::unordered_map<std::string, blockTexture> textureMapping{};
        textureData* initTextureData(
                json currTexture,
                const std::string& blockType,
                const std::string& textureName
            );
        GLuint initTexture(
                const std::string& textureName,
                const std::string& texturePath
        );
    };
}
#endif //OPENGLDEMO_TEXTURES_HPP
