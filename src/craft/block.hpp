//
// Created by admin on 5/26/2024.
//

#ifndef OPENGLDEMO_BLOCK_HPP
#define OPENGLDEMO_BLOCK_HPP
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace Craft
{
    class Block
    {
    public:
        Block(float x, float y, float z, std::string blockType, GLuint VAO, GLuint VBO);
        ~Block() = default;
        void defineSides();
        bool draw();
    private:
        std::string blockType;
        std::string textureLocation;
        GLuint topTexture;
        GLuint bottomTexture;
        GLuint frontTexture;
        GLuint rightTexture;
        GLuint backTexture;
        GLuint leftTexture;
        GLuint VAO;
        GLuint VBO;
        static json textureMapping;
        static GLuint aPosIndex;
        std::vector<float> position;
        bool initTextures();
        static GLuint initTexture(
                const std::string& textureName,
                const std::string& texturePath
        );
        void bindVAO(float* vertices, GLuint vertexCount)
    };


}

#endif //OPENGLDEMO_BLOCK_HPP
