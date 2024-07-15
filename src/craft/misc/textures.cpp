//
// Created by admin on 5/27/2024.
//

#include "textures.hpp"
#include "glm/glm.hpp"
#include <mutex>

#include "../../helpers/helpers.hpp"

namespace Craft
{
    Textures::~Textures()
    {
        for (const auto& keyValue: textureMapping)
        {
            delete keyValue.second.top;
            delete keyValue.second.bottom;
            delete keyValue.second.front;
            delete keyValue.second.right;
            delete keyValue.second.back;
            delete keyValue.second.left;
        }
    }
    blockTexture Textures::getTexture(BlockType type) const
    {
        auto it = textureMapping.find(type);
        if (it != textureMapping.end())
        {
            return it->second;
        }
        else
        {
            return blockTexture{};
        }
    }
    static std::mutex mutex{};
    void readImages(
            std::unordered_map<std::string, ImageLoadResult>& loadResults,
            json textureInfo,
            const std::string& filepath
        )
    {
        /// Iterate through texture files and get their contents.
        ImageData* imageData = getImageContents(filepath);
        if (imageData->data)
        {
            /// Create a mapping of blockType -> Faces as seen in the texture_mapping.json file
            std::unordered_map<std::string, std::vector<std::string>> blockTypeToFaces{};
            for (auto info=textureInfo.begin(); info!=textureInfo.end(); info++)
            {
                blockTypeToFaces[info.key()] = info.value();
            }
            std::lock_guard<std::mutex> lock(mutex);
            loadResults[filepath] = {0, blockTypeToFaces, imageData};
        }
        else
        {
            std::cerr << filepath << "Failed to load texture: " << filepath << std::endl;
        }
    }
    void constructTexture(
            BlockType blockType,
            const std::string& blockFace,
            GLuint texLayer,
            std::unordered_map<BlockType, blockTexture>& textureMapping
        )
    {
        glm::vec4 colorMapping{255, 255, 255, 0};
        if (blockType == BlockType::GRASS && blockFace == "top")
        {
            // Green color mapping for grass block top.
            colorMapping = glm::vec4(129, 226, 64, 204);
        }
        auto data = new textureData{texLayer, colorMapping};
        std::lock_guard<std::mutex> lock(mutex);
        blockTexture& bt = textureMapping[blockType];
        if (blockFace == "top") bt.top = data;
        else if (blockFace == "bottom") bt.bottom = data;
        else if (blockFace == "front") bt.front = data;
        else if (blockFace == "right") bt.right = data;
        else if (blockFace == "back") bt.back = data;
        else if (blockFace == "left") bt.left = data;
    }
    bool Textures::initTextures(GLuint program)
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
        std::unordered_map<std::string, ImageLoadResult> loadResults{};

        // Iterate through
        for (auto iter=textureMappingJSON.begin(); iter!=textureMappingJSON.end(); iter++)
        {
            const std::string& filepath = iter.key();
            json textureInfo = textureMappingJSON[filepath];
            threads.emplace_back([&, filepath, textureInfo]()
                {
                    readImages(loadResults, textureInfo, filepath);
                }
            );
        }
        // Start reading threads
        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        glUseProgram(program);
        GLuint tex = initTextureFromData(loadResults);
        std::cout << "Initialized Textures" << std::endl;
        glUniform1i(glGetUniformLocation(program, "textures"), 1);
        // Clearing threads.
        threads.clear();
        /// Iterate through Textures
        for (const auto& result: loadResults)
        {
            GLuint texLayer = result.second.layer;
            // Iterate through block types.
            for (const auto& info: result.second.blockTypeToFaces)
            {
                std::string blockType = info.first;
                // Iterate through block faces.
                for (const auto& blockFace: info.second)
                {
                    threads.emplace_back([&, texLayer, blockType, blockFace]()
                        {
                            constructTexture(stringToBlockType[blockType], blockFace, texLayer, textureMapping);
                        }
                    );
                }
            }
            for (auto& thread : threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
        }
        return true;
    }
    GLuint Textures::initTextureFromData(std::unordered_map<std::string, ImageLoadResult>& imageResults)
    {
        // Ensure imageResults is not empty
        if (imageResults.empty())
        {
            std::cerr << "No image results to initialize textures from" << std::endl;
            return 0;
        }

        glActiveTexture(GL_TEXTURE1);
        GLuint textureArray;
        glGenTextures(1, &textureArray);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

        auto numLayers = static_cast<GLsizei>(imageResults.size());
        std::cout << "Number of textures: " << numLayers << std::endl;

        // Allocate storage for the texture array
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 256, 256, numLayers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        int layer = 0;
        for (const auto& result : imageResults)
        {
            ImageData* imageData = result.second.imageData;
            if (imageData->data)
            {
                glTexSubImage3D(
                    GL_TEXTURE_2D_ARRAY,
                    0, 0, 0,
                    layer,
                    imageData->width,
                    imageData->height,
                    1,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    imageData->data
                );
            }
            else
            {
                std::cerr << "Failed to load texture data" << std::endl;
                return 0;
            }
            imageResults[result.first].layer = layer;
            layer++;
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return textureArray;
    }
}