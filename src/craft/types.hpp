//
// Created by admin on 5/27/2024.
//

#ifndef OPENGLDEMO_TYPES_HPP
#define OPENGLDEMO_TYPES_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include "../helpers/helpers.hpp"

struct textureData {
    GLuint id;
    glm::vec4 colorMapping;

    ~textureData() {
        glDeleteTextures(1, &id);
    }
};

struct blockTexture {
    textureData* top;
    textureData* bottom;
    textureData* front;
    textureData* right;
    textureData* back;
    textureData* left;
};

struct HasNeighbors
{
    bool top;
    bool bottom;
    bool front;
    bool right;
    bool back;
    bool left;
};

struct ImageLoadResult {
    GLuint id;
    std::unordered_map<std::string, std::vector<std::string>> blockTypeToFaces;
    Helpers::ImageData* imageData;
};

#endif //OPENGLDEMO_TYPES_HPP
