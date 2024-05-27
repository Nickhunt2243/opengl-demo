#include <glad/glad.h>
#include <iostream>

#include "program.hpp"
#include "../helpers/helpers.hpp"
namespace Engine
{
    Program::Program(
            std::string vShader,
            std::string gShader,
            std::string fShader,
            std::unordered_map<std::string, std::string>& texPathMap
        )
        : vertexShaderName( std::move(vShader) )
        , geometryShaderName( std::move(gShader) )
        , fragmentShaderName( std::move(fShader) )
        , texturePathsMap(texPathMap)
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
        // Create textures
        for (auto mapIter=texturePathsMap.begin(); mapIter!=texturePathsMap.end(); mapIter++) {
            if (!createTexture(mapIter->first, mapIter->second)) return false;
        }

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
            std::cerr << "Link info: " << infoLog << std::endl;
            return false;
        }
        glUseProgram(program);
        // Enable Depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        // Enable face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        // Clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures["grass_top"]);
        glUniform1i(glGetUniformLocation(program, "textures[0]"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures["grass_bottom"]);
        glUniform1i(glGetUniformLocation(program, "textures[1]"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures["grass_side"]);
        glUniform1i(glGetUniformLocation(program, "textures[2]"), 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures["grass_side"]);
        glUniform1i(glGetUniformLocation(program, "textures[3]"), 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textures["grass_side"]);
        glUniform1i(glGetUniformLocation(program, "textures[4]"), 4);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, textures["grass_side"]);
        glUniform1i(glGetUniformLocation(program, "textures[5]"), 5);


        return true;
    }
}