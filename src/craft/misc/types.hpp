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
    /**
     * A struct for accessing specific bits within a byte (unsigned char).
     *
     * Used to aid in the reading and writing to neighbor information.
     */
    struct ByteProxy {
        int& byte;
        int bitIndex;

        ByteProxy(int& byte, int bitIndex)
                : byte(byte), bitIndex(bitIndex) {}

        // Getter
        operator bool() const {
            return (byte >> bitIndex) & 1;
        }

        // Setter
        ByteProxy& operator=(bool value) {
            if (value)
                byte |= (1 << bitIndex);
            else
                byte &= ~(1 << bitIndex);
            return *this;
        }
    };
    /**
     * A struct for holding an entire rows (16 blocks/bytes) neighbor information.
     *
     * position 0 holds blocks 0-3
     * position 1 holds blocks 4-7
     * ...
     *
     */
    struct NeighborInfo
    {
//        int blockPosData; // Contains the x, y, z
        int sideData; // Holds information for block neighbors, and textures
//        int chunkX;
//        int chunkZ;
//        int chunkNum;
//        int blockX;
//        int blockY;
//        int blockZ;
//        int instanceID;

        void setNeighbor(int value)
        {
            sideData = (int) value;
        }
        ByteProxy block() { return {sideData, 0}; }
        bool blockExists() const {return (sideData & 1) == 1; }
        ByteProxy y_max() { return {sideData, 1}; }
        ByteProxy y_min() { return {sideData, 2}; }
        ByteProxy x_max() { return {sideData, 3}; }
        ByteProxy x_min() { return {sideData, 4}; }
        ByteProxy z_max() { return {sideData, 5}; }
        ByteProxy z_min() { return {sideData, 6}; }
        ByteProxy operator [](int index)
        {
            return {sideData, index};
        }
        [[nodiscard]] int sum() const {
            return ((sideData >> 1) & 1) + ((sideData >> 2) & 1) +
                   ((sideData >> 3) & 1) + ((sideData >> 4) & 1) +
                   ((sideData >> 5) & 1) + ((sideData >> 6) & 1);
        }

    };
    /**
     * A struct for holding the neighbor info for an entire chunk.
     *
     * A blocks neighbor information will be accessed via this method:
     *
     * rowNeighborIdx = (blockY * CHUNK_WIDTH) + blockZ
     * blockIdx = blockX
     *
     * NeighborInfo blockNeighborInfo = blockVisibility[rowNeighborIdx][blockIdx];
     */
    typedef struct
    {
        NeighborInfo blockVisibility[CHUNK_WIDTH * CHUNK_HEIGHT * 16];
    } ChunkInfoSSBO;
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
