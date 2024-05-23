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
    bool endsWith(const std::string& s, const std::string& ending)
    {
        if (ending.size() > s.size()) return false;
        auto sIter = std::prev(s.end(), 1);
        auto eIter = std::prev(ending.end(), 1);

        while (eIter > ending.begin())
        {
            std::cout << s << std::endl;
            std::cout << ending << std::endl;
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
        }
        return loc;
    }
    void setBool(GLuint program, const std::string &name, bool value)
    {
        GLint loc = getLoc(program, name);
        glUniform1i(loc, (int)value);
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
    void setVec4(GLuint program, const std::string &name, float* value)
    {
        GLint loc = getLoc(program, name);
        glUniform4fv(loc, 1, value);
    }
    void setMat4(GLuint program, const std::string &name, glm::mat4& value)
    {
        GLint loc = getLoc(program, name);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }
}