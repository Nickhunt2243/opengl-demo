//
// Created by admin on 6/6/2024.
//

#ifndef OPENGLDEMO_BLOCK_HPP
#define OPENGLDEMO_BLOCK_HPP

#include <string>
#include <ostream>
#include <unordered_set>
#include <mutex>
#include <bitset>

#include "../misc/coordinate.hpp"
#include "../misc/types.hpp"
#include "../misc/globals.hpp"

namespace Craft
{
    /**
     * A structure for holding block data.
     */
    struct Block
    {
        explicit Block(BlockType type)
            : type{type}
            , lightLevelDiff{0}
        {}
        /// The enum of the block's type.
        BlockType type{};
        /// The amount to decrease the light level by (not currently in use).
        int lightLevelDiff;
    };
    /**
     * A function that will fill the specified vPointer location with the blocks vertices.
     *
     * @param chunkRelativeCoord: The relative chunk coordinate fo the block.
     * @param currBlock:          The current block to initialize vertices for.
     * @param vPointer:           The pointer to the vertex buffer.
     * @param vIdx:               The starting idx to the block's relative buffer space.
     * @param textures:           The blocks texture.
     */
    void fillVerticesBufferData(
        Coordinate<int> chunkRelativeCoord,
        Block currBlock,
        int* vPointer, int vIdx,
        blockTexture textures
    );
    /**
     * A function for filling the element buffer's data.
     *
     * We fill the vertex buffer with all possible vertices and then we draw the vertices via the element buffer.
     * I chose this method as adding 6 GLuint per face is less data to manage compared to 40 (4 * 10) floats in the
     * vertex buffer per face.
     *
     * @param currBlock:    The current block to initialize elements for.
     * @param ePointer:     The pointer to the element buffer.
     * @param eIdx:         The starting idx of the current block's relative element buffer space.
     * @param currIndex:    The current index of the vertex to draw.
     * @param neighborInfo: The current blocks neighbor information.
     */
    void fillElementBufferData(
        uint32_t* ePointer, int eIdx,
        int currIndex, NeighborsInfo neighborInfo
    );
    /// Retrieve the number of Vertices per Cube: 10 floats per vertex * 4 per side * 6 sides.
    int getVerticesCount();
}

#endif //OPENGLDEMO_BLOCK_HPP
