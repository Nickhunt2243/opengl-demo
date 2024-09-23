#ifndef OPENGLDEMO_HELPERS_HPP
#define OPENGLDEMO_HELPERS_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <iostream>

#include "../craft/misc/types.hpp"
#include "../craft/worldGeneration/block.hpp"

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
struct ImageLoadResult
{
    /// The layer of the texture in the Sampler2DArray.
    GLuint layer;
    /// A mapping of block type to the faces the texture should be used with.
    std::unordered_map<std::string, std::vector<std::string>> blockTypeToFaces;
    /// A struct containing the ImageData.
    ImageData *imageData;
};
struct ColorMapLoadResults
{
    /// The layer of the texture in the Sampler2DArray.
    GLuint layer;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> blockTypeToFacesAndUVs;
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
 * Simple function for saving an an array of rgb data as an image. Used mostly for debugging.
 *
 * @param rgbData: The array containing the data.
 * @param width:   The width of the picture.
 * @param height:  The height of the picture.
 */
void saveImageAsPNG(unsigned char* rgbData, int width, int height);
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
 * Set a 2x1 integer vector within the OpenGL program.
 *
 * @param program: The given OpenGL program identifier.
 * @param name:    The name of the shader parameter.
 * @param value:   The value to set.
 */
void setiVec2(GLuint program, const std::string &name, glm::ivec2 value);
/**
 * Set a 3x1 vector within the OpenGL program.
 *
 * @param program: The given OpenGL program identifier.
 * @param name:    The name of the shader parameter.
 * @param value:   The value to set.
 */
void setVec3(GLuint program, const std::string &name, glm::vec3 value);
/**
 * Set a 3x1 integer vector within the OpenGL program.
 *
 * @param program: The given OpenGL program identifier.
 * @param name:    The name of the shader parameter.
 * @param value:   The value to set.
 */
void setiVec3(GLuint program, const std::string &name, glm::ivec3 value);
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
/**
 * Retrieve a given block's X, Z, and chunk coordinate normalized in 0-15 indexing.
 *
 * @param blockX:   The current block X value.
 * @param blockZ:   The current block Z value.
 * @param chunkPos: The current chunk position.
 * @return:       A struct containing the blocks x, z, and chunk coordinate.
 */
Craft::BlockInfo getBlockInfo(Craft::Coordinate<int> block, Craft::Coordinate2D<int> chunkPos);
/// Helper function for checking opengl errors.
inline void checkOpenGLError(const std::string& location) {
    std::cerr << "OpenGL Error " << glGetError() << " at " << location << std::endl;
}
/**
 * Retrieve whether a block exists in the world given its chunk and chunk relative coordinate.
 * @param info:   The blocks chunk and chunk relative coordinate.
 * @param coords: The mapping of chunks to a mapping of chunk rel block coordinates to blocks.
 * @return:       A Boolean value of whether the block exists.
 */
bool blockExists(Craft::BlockInfo info, std::unordered_map<Craft::Coordinate2D<int>, std::unordered_map<Craft::Coordinate<int>, Craft::Block>*>* coords);
/**
 * Given the x or z coordinate calculate the chunksPos between [0, TOTAL_CHUNK_WIDTH)
 *
 * Used for indexing the chunk buffer objects.
 *
 * @param coord: The X or Z Coordinate of the chunk.
 * @return:      A normalized coordinate from [0, TOTAL_CHUNK_WIDTH)
 */
int findChunkIdx(int coord);
#endif //OPENGLDEMO_HELPERS_HPP
