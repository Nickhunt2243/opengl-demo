#ifndef OPENGLDEMO_HELPERS_HPP
#define OPENGLDEMO_HELPERS_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace Helpers {
    /**
     * Retrieve the contents of a file given its path.
     *
     * @param path: The path to the file.
     * @return      A string of the contents.
     */
    std::string getFileContents(const char* path);
    /**
     * Retrieve whether the string s ends with the char sequence of ending.
     *
     * @param s:      The string to be checked.
     * @param ending: The ending sequence to use.
     * @return        True if the string ends with ending else false.
     */
    bool endsWith(const std::string& s, const std::string& ending);
    /**
     * Set a Boolean within the OpenGL program.
     *
     * @param program: The given OpenGL program identifier.
     * @param name:    The name of the shader parameter.
     * @param value:   The value to set.
     */
    void setBool(GLuint program, const std::string &name, bool value);
    /**
     * Set a Integer within the OpenGL program.
     *
     * @param program: The given OpenGL program identifier.
     * @param name:    The name of the shader parameter.
     * @param value:   The value to set.
     */
    void setInt(GLuint program, const std::string &name, int value);
    /**
     * Set a Float within the OpenGL program.
     *
     * @param program: The given OpenGL program identifier.
     * @param name:    The name of the shader parameter.
     * @param value:   The value to set.
     */
    void setFloat(GLuint program, const std::string &name, float value);
    /**
     * Set a 4x1 vector within the OpenGL program.
     *
     * @param program: The given OpenGL program identifier.
     * @param name:    The name of the shader parameter.
     * @param value:   The value to set.
     */
    void setVec4(GLuint program, const std::string &name, float* value);
    /**
     * Set a 4x4 matrix within the OpenGL program.
     *
     * @param program: The given OpenGL program identifier.
     * @param name:    The name of the shader parameter.
     * @param value:   The value to set.
     * @return         True if mat4 was set, else false
     */
    bool setMat4(GLuint program, const std::string &name, glm::mat4& value);
}


#endif //OPENGLDEMO_HELPERS_HPP
