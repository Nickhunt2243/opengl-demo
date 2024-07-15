//
// Created by admin on 6/6/2024.
//

#ifndef OPENGLDEMO_BLOCK_HPP
#define OPENGLDEMO_BLOCK_HPP

#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 16
#endif
#ifndef CHUNK_HEIGHT
#define CHUNK_HEIGHT 128
#endif

#include <string>
#include <ostream>
#include <unordered_set>
#include <mutex>
#include <bitset>

#include "../misc/coordinate.hpp"
#include "../misc/types.hpp"

namespace Craft
{
    /**
     * A structure for holding block data.
     */
    struct Block
    {
        Block(const Coordinate<int>& coord, blockTexture textures, BlockType type)
            : chunkRelativeCoord{coord}
            , textures{textures}
            , type{type}
            , lightLevelDiff{0}
        {}
        /// The position of the chunk the coord is in.
        Coordinate<int> chunkRelativeCoord;
        /// The information of the blocks neighbors.
        NeighborsInfo neighborInfo{};
        /// The enum of the block's type.
        BlockType type{};
        /// The information for the blocks textures and color mapping.
        blockTexture textures{};
        /// The amount to decrease the light level by (not currently in use).
        int lightLevelDiff;
    };
    /**
     * A function for determining if a block is next to another block.
     *
     * Only ran for the inner blocks <X: [1, 14], Z: [1, 14], Y: [0, 128]>
     *
     * @param blockCoords: A set containing all of the coordinates hashed together.
     *                     For the hash function see coordinate.hpp
     * @param x:           The x position of the block (relative to chunk space.)
     * @param y:           The y position of the block (relative to chunk space.)
     * @param z:           The z position of the block (relative to chunk space.)
     * @param currBlock:   The current block to be checked.
     */
    void updateNeighbors(
        std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> blockCoords,
        int x, int y, int z,
        Block* currBlock
    );
    /**
     * A function that will fill the specified vPointer location with the blocks vertices.
     *
     * @param currBlock: The current block to initialize vertices for.
     * @param vPointer:  The pointer to the vertex buffer.
     * @param vIdx:      The starting idx to the block's relative buffer space.
     */
    void fillVerticesBufferData(
        Block* currBlock,
        int* vPointer, int vIdx
    );
    /**
     * A function for filling the element buffer's data.
     *
     * We fill the vertex buffer with all possible vertices and then we draw the vertices via the element buffer.
     * I chose this method as adding 6 GLuint per face is less data to manage compared to 40 (4 * 10) floats in the
     * vertex buffer per face.
     *
     * @param currBlock: The current block to initialize elements for.
     * @param ePointer:  The pointer to the element buffer.
     * @param eIdx:      The starting idx of the current block's relative element buffer space.
     * @param currIndex: The current index of the vertex to draw.
     */
    void fillElementBufferData(
        Block* currBlock,
        unsigned int* ePointer, int eIdx,
        int currIndex
    );
    /// Retrieve the number of Vertices per Cube: 10 floats per vertex * 4 per side * 6 sides.
    int getVerticesCount();
    /// Retrieve the number of Elements needed to draw a Cube: 6 per side * visible sides [0,6]
    int getElementSize(Block* currBlock);
    /**
     * Determine whether a block on the left edge has a block next to it in the next chunk.
     *
     * @param leftChunkBlockCoords: The blockCoords of the chunk to the left.
     * @param x:                    The x position of the current block (relative to chunk space.)
     * @param y:                    The y position of the current block (relative to chunk space.)
     * @param z:                    The z position of the current block (relative to chunk space.)
     * @param currBlock:            The current block to be checked.
     * @return:                     True if the neighbor was updates, false if the neighbor is the same.
     */
    bool updateLeftEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> leftChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
    );
    /**
     * Determine whether a block on the right edge has a block next to it in the next chunk.
     *
     * @param rightChunkBlockCoords: The blockCoords of the chunk to the right.
     * @param x:                     The x position of the current block (relative to chunk space.)
     * @param y:                     The y position of the current block (relative to chunk space.)
     * @param z:                     The z position of the current block (relative to chunk space.)
     * @param currBlock:             The current block to be checked.
     * @return:                      True if the neighbor was updates, false if the neighbor is the same.
     */
    bool updateRightEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> rightChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
    );
    /**
     * Determine whether a block on the front edge has a block next to it in the next chunk.
     *
     * @param frontChunkBlockCoords: The blockCoords of the chunk to the front.
     * @param x:                     The x position of the current block (relative to chunk space.)
     * @param y:                     The y position of the current block (relative to chunk space.)
     * @param z:                     The z position of the current block (relative to chunk space.)
     * @param currBlock:             The current block to be checked.
     * @return:                      True if the neighbor was updates, false if the neighbor is the same.
     */
    bool updateFrontEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> frontChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
    );
    /**
     * Determine whether a block on the back edge has a block next to it in the next chunk.
     *
     * @param backChunkBlockCoords: The blockCoords of the chunk to the back.
     * @param x:                    The x position of the current block (relative to chunk space.)
     * @param y:                    The y position of the current block (relative to chunk space.)
     * @param z:                    The z position of the current block (relative to chunk space.)
     * @param currBlock:            The current block to be checked.
     * @return:                     True if the neighbor was updates, false if the neighbor is the same.
     */
    bool updateBackEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> backChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
    );
}

#endif //OPENGLDEMO_BLOCK_HPP
