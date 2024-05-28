//
// Created by admin on 5/27/2024.
//

#include "textures.hpp"
#include <glm/glm.hpp>
#include <mutex>

namespace Craft
{
    Textures::~Textures() {
        for (const auto& keyValue: textureMapping) {
            delete keyValue.second.top;
            delete keyValue.second.bottom;
            delete keyValue.second.front;
            delete keyValue.second.right;
            delete keyValue.second.back;
            delete keyValue.second.left;
        }
    }
    blockTexture Textures::getTexture(std::string& name)
    {
        return textureMapping[name];
    }
    static std::mutex mutex{};
    void readImages(
            std::unordered_map<std::string, ImageLoadResult>& loadResults,
            json textureInfo,
            std::string filepath
        )
    {
        /// Iterate through texture files and get their contents.
        Helpers::ImageData* imageData = Helpers::getImageContents(filepath);
        if (imageData->data) {
            /// Create a mapping of blockType -> Faces as seen in the texture_mapping.json file
            std::unordered_map<std::string, std::vector<std::string>> blockTypeToFaces{};
            for (auto info=textureInfo.begin(); info!=textureInfo.end(); info++) {
                blockTypeToFaces[info.key()] = info.value();
            }
            std::lock_guard<std::mutex> lock(mutex);
            loadResults[filepath] = {0, blockTypeToFaces, imageData};
        } else {
            std::cerr << filepath << "Failed to load texture: " << filepath << std::endl;
        }
    }
    void constructTexture(
            const std::string& blockType,
            const std::string& blockFace,
            GLuint texId,
            std::unordered_map<std::string, blockTexture>& textureMapping
        )
    {
        glm::vec4 colorMapping{1.0, 1.0, 1.0, 0.0};
        if (blockType == "grass" && blockFace == "top") {
            colorMapping = glm::vec4(0.5065, 0.8296, 0.2516, 0.8);
        }
        auto data = new textureData{texId, colorMapping};
        std::lock_guard<std::mutex> lock(mutex);
        blockTexture& bt = textureMapping[blockType];
        if (blockFace == "top") bt.top = data;
        else if (blockFace == "bottom") bt.bottom = data;
        else if (blockFace == "front") bt.front = data;
        else if (blockFace == "right") bt.right = data;
        else if (blockFace == "back") bt.back = data;
        else if (blockFace == "left") bt.left = data;
    }
    bool Textures::initTextures()
    {
        std::cout << "Initializing textures." << std::endl;
        std::filesystem::path current_path = std::filesystem::current_path();
        std::filesystem::path file_path = current_path.parent_path() / "src/assets/json/texture_mapping.json";
        std::ifstream file(file_path);
        // Define the path to the JSON file
        std::unordered_map<std::string, GLuint> createdTexts{};
        std::ifstream f(file_path);
        json textureMappingJSON = json::parse(f);
        std::vector<std::thread> threads;
//        std::mutex mutex{};
        std::unordered_map<std::string, ImageLoadResult> loadResults{};

        // Iterate through
        for (auto iter=textureMappingJSON.begin(); iter!=textureMappingJSON.end(); iter++) {
            const std::string& filepath = iter.key();
            json textureInfo = textureMappingJSON[filepath];
            threads.emplace_back([&, filepath, textureInfo]() {
                readImages(loadResults, textureInfo, filepath);
            });
        }
        // Start reading threads
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Create the OpenGL textures outside of threads.
        for (const auto& result : loadResults) {
            GLuint tex = initTextureFromData(result.second.imageData);
            std::cout << "Tex: " << tex << std::endl;
            loadResults[result.first].id = tex;
        }
        // Clearing threads.
        threads.clear();
        /// Iterate through Textures
        for (const auto& result: loadResults) {
            GLuint texId = result.second.id;
            // Iterate through block types.
            for (const auto& info: result.second.blockTypeToFaces) {
                std::string blockType = info.first;
                // Iterate through block faces.
                for (const auto& blockFace: info.second) {
                    threads.emplace_back([&, texId, blockType, blockFace]() {
                        constructTexture(blockType, blockFace, texId, textureMapping);
                    });
                }
            }
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }




        return true;
    }
    textureData* Textures::initTextureData(json currTexture, const std::string& blockType, const std::string& textureName) {
        auto data = new textureData();
        GLuint tex = initTexture(textureName, currTexture[textureName]);
        std::cout << "Tex: " << tex << std::endl;
        data->id = tex;
        if (blockType == "grass" && textureName == "top") {
            // Quick fix for now to apply colorMapping to only the grass's top face
            data->colorMapping = glm::vec4(0.5065, 0.8296, 0.2516, 0.8);
        } else {
            data->colorMapping = glm::vec4(1.0, 1.0, 1.0, 0.0);
        }

        return data;
    }

    GLuint Textures::initTextureFromData(Helpers::ImageData* imageData)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (imageData->data)
        {
            GLint internalFormat;
            GLenum format;
            if (imageData->nrChannels == 1) {
                internalFormat = GL_RED;
                format = GL_RED;
            }
            else if (imageData->nrChannels == 3) {
                internalFormat = GL_RGB;
                format = GL_RGB;
            }
            else if (imageData->nrChannels == 4) {
                internalFormat = GL_RGBA;
                format = GL_RGBA;
            } else {
                std::cerr << "Unsupported number of channels: " << imageData->nrChannels << std::endl;
                return false;
            }
            glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    internalFormat,
                    imageData->width,
                    imageData->height,
                    0,
                    format,
                    GL_UNSIGNED_BYTE,
                    imageData->data
            );
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cerr << "Failed to load texture data" << std::endl;
            return false;
        }

        return texture;
    }

    GLuint Textures::initTexture(
            const std::string& textureName,
            const std::string& texturePath
        )
    {
        GLuint texture;
        glGenTextures(1, &texture);
        std::cout << glGetError() << std::endl;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        Helpers::ImageData* imageData = Helpers::getImageContents(texturePath);

        if (imageData->data)
        {
            GLint internalFormat;
            GLenum format;
            if (imageData->nrChannels == 1) {
                internalFormat = GL_RED;
                format = GL_RED;
            }
            else if (imageData->nrChannels == 3) {
                internalFormat = GL_RGB;
                format = GL_RGB;
            }
            else if (imageData->nrChannels == 4) {
                internalFormat = GL_RGBA;
                format = GL_RGBA;
            } else {
                std::cerr << "Unsupported number of channels: " << imageData->nrChannels << std::endl;
                return false;
            }
            glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    internalFormat,
                    imageData->width,
                    imageData->height,
                    0,
                    format,
                    GL_UNSIGNED_BYTE,
                    imageData->data
            );
        }
        else
        {
            std::cerr << "Failed to load "<< textureName << " texture: " << texturePath << std::endl;
            delete imageData;
            return false;
        }
        delete imageData;
        return texture;
    }

}