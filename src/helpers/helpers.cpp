#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>


#include "helpers.hpp"

namespace Helpers
{
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
                0
        );

        return imgData;
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
    template <typename K, typename V>
    std::unordered_set<K> createSetFromMapKeys(const std::unordered_map<K, V>& map) {
        std::unordered_set<K> keySet;
        for (const auto& pair : map) {
            keySet.insert(pair.first);
        }
        return keySet;
    }
}