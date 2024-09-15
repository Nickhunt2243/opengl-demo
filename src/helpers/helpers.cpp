#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "helpers.hpp"

std::string getFileContents(const char* path)
{
    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path file_path = current_path.parent_path() / path;
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
ImageData* getImageContents(const std::string& texturePath)
{
    auto imgData = new ImageData();
    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path file_path = current_path.parent_path() / texturePath;
    std::string fileString = file_path.string();
    char const* fileCharConst = fileString.c_str();

    imgData->data = stbi_load(
            fileCharConst,
            &imgData->width,
            &imgData->height,
            &imgData->nrChannels,
            STBI_rgb_alpha
    );
    if (imgData->data && imgData->nrChannels != 4)
    {
        std::cout << "Path: " << texturePath << " Channels: " << imgData->nrChannels << std::endl;
        imgData->nrChannels = 4;
    }

    return imgData;
}
void saveImageAsPNG(unsigned char* rgbaData, int width, int height)
{
    const std::string& filename = R"(C:\Users\admin\CLionProjects\opengl-demo\test.png)";
    // Save the image data to a PNG file using stb_image_write
    int result = stbi_write_png(filename.c_str(), width, height, 4, rgbaData, width * 4);

    if (result == 0)
    {
        std::cerr << "Failed to save image to " << filename << std::endl;
    }
    else
    {
        std::cout << "Image saved successfully to " << filename << std::endl;
    }
}
bool endsWith(const std::string& s, const std::string& ending)
{
    if (ending.size() > s.size()) return false;
    auto sIter = std::prev(s.end(), 1);
    auto eIter = std::prev(ending.end(), 1);

    while (eIter > ending.begin())
    {
        if (*sIter != *eIter)
        {
            return false;
        }
        --sIter;
        --eIter;
    }

    return true;
}
GLint getLoc( GLuint program, const std::string& name )
{
    GLint loc = glGetUniformLocation(program, name.c_str());
    if (loc == -1)
    {
        std::cout << "Failed to find location: " << name << std::endl;
        return -1;
    }
    return loc;
}
void setBool(GLuint program, const std::string &name, bool value)
{
    GLint loc = getLoc(program, name);
    glUniform1i(loc, value ? 1 : 0);
}
void setInt(GLuint program, const std::string &name, int value)
{
    GLint loc = getLoc(program, name);
    glUniform1i(loc, value);
}
void setFloat(GLuint program, const std::string &name, float value)
{
    GLint loc = getLoc(program, name);
    glUniform1f(loc, value);
}
void setVec2(GLuint program, const std::string &name, glm::vec2 value)
{
    GLint loc = getLoc(program, name);
    glUniform2fv(loc, 1, glm::value_ptr(value));
}
void setiVec2(GLuint program, const std::string &name, glm::ivec2 value)
{
    GLint loc = getLoc(program, name);
    glUniform2iv(loc, 1, glm::value_ptr(value));
}
void setVec3(GLuint program, const std::string &name, glm::vec3 value)
{
    glUseProgram(program);
    GLint loc = getLoc(program, name);
    glUniform3fv(loc, 1, glm::value_ptr(value));
}
void setiVec3(GLuint program, const std::string &name, glm::ivec3 value)
{
    glUseProgram(program);
    GLint loc = getLoc(program, name);
    glUniform3iv(loc, 1, glm::value_ptr(value));
}
void setVec4(GLuint program, const std::string &name, glm::vec4 value)
{
    GLint loc = getLoc(program, name);
    glUniform4fv(loc, 1, glm::value_ptr(value));
}
bool setMat4(GLuint program, const std::string &name, glm::mat4& value)
{
    GLint loc = getLoc(program, name);
    if (loc == -1) {
        return false;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    return true;
}
void printMatrix4x4(glm::mat4& mat, const std::string& name)
{
    std::cout << "\n" << name << " Matrix:" << std::endl;
    std::cout << "{\n\t";
    glm::mat4 trans = glm::transpose(mat);
    for (int i=0; i<16;i++)
    {
        std::cout << glm::value_ptr(trans)[i] << " ";
        if (i % 4 == 3 && i != 15) std::cout << "\n\t";
    }
    std::cout << "\n}" << std::endl;
}

Craft::BlockInfo getBlockInfo(Craft::Coordinate<int> block, Craft::Coordinate2D<int> chunkPos)
{
    Craft::BlockInfo info
            {
                    block,
                    chunkPos
            };
    if (block.z < 0)
    {
        info.block.z += 16;
        info.chunk.z -= 1;
    }
    if (block.z > 15)
    {
        info.block.z -= 16;
        info.chunk.z += 1;
    }
    if (block.x < 0)
    {
        info.block.x += 16;
        info.chunk.x -= 1;
    }
    if (block.x > 15)
    {
        info.block.x -= 16;
        info.chunk.x += 1;
    }
    return info;
}
bool blockExists(
        Craft::BlockInfo info,
        std::unordered_map<Craft::Coordinate2D<int>, std::unordered_map<Craft::Coordinate<int>, Craft::Block>*>* coords
    )
{
    auto chunkIter = coords->find(info.chunk);
    if (chunkIter == coords->end()) return false;
    return chunkIter->second->find(info.block) != chunkIter->second->end();
}