//
// Created by admin on 8/25/2024.
//

#ifndef OPENGLDEMO_COMPUTE_HPP
#define OPENGLDEMO_COMPUTE_HPP
#include <glad/glad.h>
#include <string>
#include <unordered_map>

namespace Engine
{
    class Compute
    {
    public:
        explicit Compute(
                std::string cShader
        );
        ~Compute();
        /**
         * Retrieve the OpenGL program.
         *
         * @return The Identifier of the OpenGL program
         */
        [[nodiscard]] GLuint getProgram() const;
        /// Use the specified OpenGL Program.
        void useCompute() const;
        /**
         * Initialize the OpenGL program and compile the compute shader.
         *
         * Creates an OpenGL program, compiles and attaches the specified compute shader, and finally link the program.
         *
         * @return: True if the program was initialized, else False.
         */
        bool initCompute();
    private:
        /// The path of the fragment shader.
        std::string computeShaderName;
        /// The identifier of the compute shader.
        GLint compShader{0};
        /// The identifier of the OpenGL program.
        GLuint program{0};
        /**
         * Compile the shader code given a path.
         *
         * Read in shader code from file, compile it, then attach it to the program.
         *
         * @param shaderPath: The path to the shader.
         * @return            true if successful else false.
         */
        bool genCompute(const std::string& computePath);
    };
}
#endif //OPENGLDEMO_COMPUTE_HPP
