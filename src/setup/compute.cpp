#include <glad/glad.h>
#include <iostream>

#include "compute.hpp"
#include "../helpers/helpers.hpp"
namespace Engine
{
    Compute::Compute(
            std::string cShader
    )
        : computeShaderName( std::move(cShader) )
    {};

    Compute::~Compute()
    {
        glDeleteProgram(compShader);
        glDeleteProgram(program);
    }
    GLuint Compute::getProgram() const
    {
        return program;
    }
    void Compute::useCompute() const
    {
        glUseProgram(program);
    }
    bool Compute::genCompute(const std::string& shaderPath)
    {
        std::string shaderSource = getFileContents(shaderPath.c_str());
        if (shaderSource.empty())
        {
            std::cerr << "Shader contains no content: " << shaderPath << std::endl;
        }
        const char* shaderSrc = shaderSource.c_str();
        int success;
        char infoLog[512];

        std::cout << "Generating Compute Shader: " << shaderPath << std::endl;
        compShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compShader, 1, &shaderSrc, nullptr);
        glCompileShader(compShader);
        glGetShaderiv(compShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(compShader, 512, nullptr, infoLog);
            std::string msg = "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n";
            std::cerr << msg << infoLog << std::endl;
            return false;
        }
        return true;
    }

    bool Compute::initCompute()
    {
        program = glCreateProgram();


        int success;
        char infoLog[512];
        if (!genCompute(computeShaderName)) return false;
        glAttachShader(program, compShader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success)
        {
            std::cerr << "OpenGL Error: " << glGetError() << std::endl;
            std::cerr << "Failed to link the program." << std::endl;
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Link info: " << infoLog << std::endl;
            return false;
        }
        glUseProgram(program);
        return true;
    }
}