#ifndef OPENGLDEMO_HELPERS_HPP
#define OPENGLDEMO_HELPERS_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>

#include "../craft/misc/types.hpp"

/// A struct containing information about a loaded image.
struct ImageData {
    /// The width of the image.
    int width;
    /// The height of the image.
    int height;
    /// The number of channels the image has.
    int nrChannels;
    /// The data of the image.
    unsigned char *data;

    ~ImageData() {
        stbi_image_free(data);
    }
};
/**
 * A struct to hold information on an images texture
 */
struct ImageLoadResult {
    /// The layer of the texture in the Sampler2DArray.
    GLuint layer;
    /// A mapping of block type to the faces the texture should be used with.
    std::unordered_map<std::string, std::vector<std::string>> blockTypeToFaces;
    /// A struct containing the ImageData.
    ImageData *imageData;
};
/**
 * Retrieve the contents of a file given its path.
 *
 * @param path: The path to the file.
 * @return      A string of the contents.
 */
std::string getFileContents(const char* path);
/**
 * Retrieve the Information of the image at the given path.
 *
 * @param texturePath: The relative path from src, to the image.
 * @return:            A struct containing the information on the image.
 *
 * @see ImageData
 */
ImageData* getImageContents(const std::string& texturePath);
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
 * Set a 2x1 vector within the OpenGL program.
 *
 * @param program: The given OpenGL program identifier.
 * @param name:    The name of the shader parameter.
 * @param value:   The value to set.
 */
void setVec2(GLuint program, const std::string &name, glm::vec2 value);
/**
 * Set a 4x1 vector within the OpenGL program.
 *
 * @param program: The given OpenGL program identifier.
 * @param name:    The name of the shader parameter.
 * @param value:   The value to set.
 */
void setVec4(GLuint program, const std::string &name, glm::vec4 value);
/**
 * Set a 4x4 matrix within the OpenGL program.
 *
 * @param program: The given OpenGL program identifier.
 * @param name:    The name of the shader parameter.
 * @param value:   The value to set.
 * @return         True if mat4 was set, else false
 */
bool setMat4(GLuint program, const std::string &name, glm::mat4& value);
/**
 * Print a 4x4 matrix for debugging.
 *
 * @param mat:  The matrix to be printed.
 * @param name: The Name/Desc of the matrix.
 */
[[maybe_unused]] void printMatrix4x4(glm::mat4& mat, const std::string& name);


#endif //OPENGLDEMO_HELPERS_HPP
