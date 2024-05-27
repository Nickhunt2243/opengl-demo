//
// Created by admin on 5/26/2024.
//

#include "block.hpp"
#include <stb/stb_image.h>
#include <filesystem>
#include <iostream>
#include <fstream>


namespace Craft
{
    GLuint Block::aPosIndex = 0;
    json Block::textureMapping = nullptr;

    Block::Block(
            float x,
            float y,
            float z,
            std::string blockType,
            GLuint VAO,
            GLuint VBO
    )
        : position( {x, y, z} )
        , blockType( std::move(blockType) )
        , VAO( VAO )
        , VBO( VBO )
    {
        initTextures();
    }

    bool Block::initTextures()
    {
        if (!textureMapping) {
            std::filesystem::path current_path = std::filesystem::current_path();
            std::filesystem::path file_path = current_path.parent_path() / "src/assets/json/texture_mapping.json";
            std::ifstream file(file_path);
            // Define the path to the JSON file
            std::ifstream f(file_path);
            textureMapping = json::parse(f);
            std::cout << to_string(textureMapping) << std::endl;
        }
        json currTexture = textureMapping[blockType];

        topTexture = initTexture("top", currTexture["top"]);
        bottomTexture = initTexture("bottom", currTexture["bottom"]);
        frontTexture = initTexture("front", currTexture["front"]);
        rightTexture = initTexture("right", currTexture["right"]);
        backTexture = initTexture("back", currTexture["back"]);
        leftTexture = initTexture("left", currTexture["left"]);

        return true;
    }

    GLuint Block::initTexture(
            const std::string& textureName,
            const std::string& texturePath
        )
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int width, height, nrChannels;
        std::filesystem::path current_path = std::filesystem::current_path();
        std::filesystem::path file_path = current_path.parent_path() / texturePath;
        std::string fileString = file_path.string();
        char const* fileCharConst = fileString.c_str();
        unsigned char *data = stbi_load(
                fileCharConst,
                &width,
                &height,
                &nrChannels,
                0
        );
        if (data)
        {
            std::cout << "Successfully loaded texture: " << texturePath << std::endl;
            std::cout << "Width: " << width << ", Height: " << height << ", Channels: " << nrChannels << std::endl;
            GLint internalFormat;
            GLenum format;
            if (nrChannels == 1) {
                internalFormat = GL_RED;
                format = GL_RED;
            }
            else if (nrChannels == 3) {
                internalFormat = GL_RGB;
                format = GL_RGB;
            }
            else if (nrChannels == 4) {
                internalFormat = GL_RGBA;
                format = GL_RGBA;
            } else {
                std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
                stbi_image_free(data);
                return false;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cerr << "Failed to load "<< textureName << " texture: " << texturePath << std::endl;
            return false;
        }

        stbi_image_free(data);
        return texture;
    }
    void Block::bindVAO(float* vertices, GLuint vertexCount)
    {
        // Bind, enable, and initialize the VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,  vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(aPosIndex);
        glVertexAttribPointer(aPosIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        // Unbind all buffers
        glBindVertexArray(0); // Unbind VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    }
    void defineSides() {

    }
    bool Block::draw()
    {
        glBindVertexArray(VAO);
        bindVAO(position.data(), position.size());
        glDrawArrays(GL_POINTS, 0, 1);
        return true;
    }
}