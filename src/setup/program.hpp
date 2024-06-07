#ifndef OPENGLDEMO_PROGRAM_HPP
#define OPENGLDEMO_PROGRAM_HPP
#include <string>
#include <unordered_map>

namespace Engine
{
    class Program
    {
    public:
        explicit Program(
            std::string vShader,
            std::string gShader,
            std::string fShader
        );
        ~Program();
        /**
         * Retrieve the OpenGL program.
         *
         * @return The Identifier of the OpenGL program
         */
        [[nodiscard]] GLuint getProgram() const;
        /// Use the specified OpenGL Program.
        void useProgram() const;
        /**
         * Initialize the OpenGL program.
         *
         * Creates an OpenGL program, compiles and attaches the specified shaders, and finally links the program.
         *
         * @return: True if the program was initialized, else False.
         */
        bool initProgram();
    private:
        /// The path of the vertex shader.
        std::string vertexShaderName;
        /// The path of the geometry shader.
        std::string geometryShaderName;
        /// The path of the fragment shader.
        std::string fragmentShaderName;
        /// A vector containing paths to all texture files.
        std::unordered_map<std::string, std::string> texturePathsMap;
        /// The identifier of the vertex shader.
        GLint vertShader{0};
        /// The identifier of the geometry shader.
        GLint geomShader{0};
        /// The identifier of the fragment shader.
        GLint fragShader{0};
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
        bool genShader(const std::string& shaderPath);
    };
}
#endif //OPENGLDEMO_PROGRAM_HPP