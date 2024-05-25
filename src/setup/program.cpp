#include <glad/glad.h>
#include <iostream>

#include "program.hpp"
#include "../helpers/helpers.hpp"

namespace Engine
{
    Program::Program(std::string vShader, std::string gShader, std::string fShader)
        : vertexShaderName( std::move(vShader) )
        , geometryShaderName( std::move(gShader) )
        , fragmentShaderName( std::move(fShader) )
    {};

    Program::~Program()
    {
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(geomShader);
        glDeleteShader(fragShader);
    }
    unsigned int Program::getProgram() const
    {
        return program;
    }
    void Program::useProgram() const
    {
        glUseProgram(program);
    }
    bool Program::genShader(const std::string& shaderPath)
    {
        std::string shaderSource = Helpers::getFileContents(shaderPath.c_str());
        if (shaderSource.empty())
        {
            std::cerr << "Shader contains no content: " << shaderPath << std::endl;
        }
        const char* shaderSrc = shaderSource.c_str();
        int success;
        char infoLog[512];
        bool isVertShader = Helpers::endsWith(shaderPath, ".vert");
        bool isGeomShader = Helpers::endsWith(shaderPath, ".geom");

        GLint shader;
        if (isVertShader)
        {
            std::cout << "Generating Geometry Shader: " << shaderPath << std::endl;
            vertShader = glCreateShader(GL_VERTEX_SHADER);
            shader = vertShader;
        }
        else if (isGeomShader)
        {
            std::cout << "Generating Geometry Shader: " << shaderPath << std::endl;
            geomShader = glCreateShader(GL_GEOMETRY_SHADER);
            shader = geomShader;
        }
        else
        {
            std::cout << "Generating Fragment Shader: " << shaderPath << std::endl;
            fragShader = glCreateShader(GL_FRAGMENT_SHADER);
            shader = fragShader;
        }
        glShaderSource(shader, 1, &shaderSrc, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::string msg = isVertShader
                              ? "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                              : isGeomShader
                              ? "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n"
                              : "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n";
            std::cerr << msg << infoLog << std::endl;
            return false;
        }

        return true;
    }

    bool Program::initProgram()
    {
        program = glCreateProgram();
        if (!genShader(vertexShaderName)) return false;
        if (!genShader(geometryShaderName)) return false;
        if (!genShader(fragmentShaderName)) return false;
        int success;
        char infoLog[512];
        glAttachShader(program, vertShader);
        glAttachShader(program, geomShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success)
        {
            std::cerr << "OpenGL Error: " << glGetError() << std::endl;
            std::cerr << "Failed to link the program." << std::endl;
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            return false;
        }
        return true;
    }
}