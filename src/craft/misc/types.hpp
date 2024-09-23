#ifndef OPENGLDEMO_TYPES_HPP
#define OPENGLDEMO_TYPES_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

#include "coordinate.hpp"
#include "globals.hpp"

// Forward declarations to avoid circular inclusions.
namespace Engine
{
    class Camera;
}

namespace Craft
{
    /// A struct for holding texture data.
    struct textureData {
        /// The layer of the texture in the sampler2DArray
        GLuint layer;
        ~textureData() {
            glDeleteTextures(1, &layer);
        }
    };

    /// A struct holding relevant block info for x/z block collision.
    struct BlockInfo {
        BlockInfo(Coordinate<int> blockPos, Coordinate2D<int> chunkPos): block{blockPos}, chunk{chunkPos} {}
        Coordinate<int> block;
        Coordinate2D<int> chunk;
    };
    /// A struct holding information about the in game time.
    struct Time {
        int hours;
        int minutes;
        float seconds;
        [[nodiscard]] float getFullTime() const
        {
            return (float) hours + ((float) minutes / 60) + (seconds / 3600.0f);
        }
    };

    /// A struct containing all the information of the block's textures.
    struct BlockTexture {
        textureData *top;
        textureData *bottom;
        textureData *front;
        textureData *right;
        textureData *back;
        textureData *left;

        textureData* operator [](int index) const
        {
            switch (index)
            {
                case 0: return top;
                case 1: return bottom;
                case 2: return front;
                case 3: return right;
                case 4: return back;
                case 5: return left;
                default: return top;
            }
        }
    };
    typedef struct {
        GLuint count;
        GLuint instanceCount;
        GLuint first;
        GLuint baseInstance;
    } DrawArraysIndirectCommand;
    struct NeighborInfo
    {
        int sideData; // Holds information for block neighbors, and textures
        int lighting[3]; // Holds information for the blocks ambient occlusion.
    };
    /**
     * A enum class of types of block's.
     */
    enum class BlockType {
        GRASS,
        DIRT,
        STONE
    };
    /// An enum denoting every side of a block.
    enum class BlockSideType {
        X_MAX,
        X_MIN,
        Y_MAX,
        Y_MIN,
        Z_MAX,
        Z_MIN,
        // For not looking at blocks
        NONE
    };
    /**
     * The << operator for the enum class BlockType for troubleshooting.
     *
     * @param os:        The ostream to append the text to.
     * @param blockType: The enum type of the block.
     * @return:          The updated ostream.
     */
    extern std::ostream &operator<<(std::ostream &os, BlockType blockType);
    /// A mapping of lowercase block names to their type in the enum class BlockType
    extern std::unordered_map<std::string, BlockType> stringToBlockType;
    /// Forward declaration for World class.
    class World;
    /// A struct for our userPointer information. Needed for GLFW Mouse callbacks.
    struct GLFWUserPointer {
        Engine::Camera* camera = nullptr;
        World* world = nullptr;
    };
}

#endif //OPENGLDEMO_TYPES_HPP
