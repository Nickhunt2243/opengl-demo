//
// Created by admin on 5/27/2024.
//

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
        /// The color mapping of the texture.
        glm::vec4 colorMapping;
        ~textureData() {
            glDeleteTextures(1, &layer);
        }
    };

    /// A struct holding relevant block info for x/z block collision.
    struct BlockInfo {
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
    struct blockTexture {
        textureData *top;
        textureData *bottom;
        textureData *front;
        textureData *right;
        textureData *back;
        textureData *left;
    };
    /**
     * A struct for accessing specific bits within a byte (unsigned char).
     *
     * Used to aid in the reading and writing to neighbor information.
     */
    struct ByteProxy {
        unsigned char& byte;
        int bitIndex;

        ByteProxy(unsigned char& byte, int bitIndex)
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
    /// A struct containing information on a block's neighbors.
    struct NeighborsInfo {
        /**
         * Bits are layed out as such:
         * 0 -> blockExists: Boolean of whether the block exists.
         * 1 -> y_max:       Boolean of whether we will draw the +1 Y side of the block.
         * 2 -> y_min:       Boolean of whether we will draw the -1 Y side of the block.
         * 3 -> x_max:       Boolean of whether we will draw the +1 X side of the block.
         * 4 -> x_min:       Boolean of whether we will draw the -1 X side of the block.
         * 5 -> z_max:       Boolean of whether we will draw the +1 Z side of the block.
         * 6 -> z_min:       Boolean of whether we will draw the -1 Z side of the block.
         *
         */
        unsigned char data;

        NeighborsInfo() : data{0} {}
        NeighborsInfo(unsigned char data) : data{data} {}

        // Accessor methods using ByteProxy
        ByteProxy block() { return {data, 0}; }
        bool blockExists() const {return (data & 1) == 1; }
        ByteProxy y_max() { return {data, 1}; }
        ByteProxy y_min() { return {data, 2}; }
        ByteProxy x_max() { return {data, 3}; }
        ByteProxy x_min() { return {data, 4}; }
        ByteProxy z_max() { return {data, 5}; }
        ByteProxy z_min() { return {data, 6}; }
        ByteProxy operator [](int index)
        {
            return {data, index};
        }
        [[nodiscard]] int sum() const {
            return ((data >> 1) & 1) + ((data >> 2) & 1) +
                   ((data >> 3) & 1) + ((data >> 4) & 1) +
                   ((data >> 5) & 1) + ((data >> 6) & 1);
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
    struct RowNeighborInfo
    {
        glm::ivec4 rowNeighborInfo;

        void setNeighbor(int index, NeighborsInfo value)
        {
            int newValue = ((int) value.data << (8 * (3 - (index % 4))));
            int clearMask = ~(0xFF << (8 * (3 - (index % 4))));
            rowNeighborInfo[index / 4] &= clearMask;
            rowNeighborInfo[index / 4] |= newValue;
        }

        NeighborsInfo operator [](int index)
        {
            int elementIndex = index / 4;
            int byteOffset = 3 - (index % 4);

            int ele = rowNeighborInfo[elementIndex];
            return (NeighborsInfo) (ele >> (8 * (byteOffset)) & 255);
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
    struct ChunkInfoSSBO
    {
        RowNeighborInfo blockVisibility[CHUNK_WIDTH * CHUNK_HEIGHT];
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
