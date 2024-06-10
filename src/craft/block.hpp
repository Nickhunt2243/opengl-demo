//
// Created by admin on 6/6/2024.
//

#ifndef OPENGLDEMO_BLOCK_HPP
#define OPENGLDEMO_BLOCK_HPP

#define CUBE_DELTA 0.5f

#include <string>
#include <ostream>
#include <unordered_set>

#include "coordinate.hpp"
#include "types.hpp"

namespace Craft
{
    /**
     * A structure for holding block data.
     */
    struct Block
    {
        /// The id of the current block in the buffer
        unsigned int id;
        /// The Coordinate struct of the blocks position.
        Coordinate* coord{nullptr};
        /// The enum of the block's type.
        BlockType type{};
        /// The information of the blocks neighbors.
        NeighborsInfo neighborInfo{};
        /// The information for the blocks textures and color mapping.
        blockTexture textures{};
        ~Block()
        {
            delete coord;
        }
        /// << Operator for troubleshooting.
        friend std::ostream& operator<<(std::ostream &os, const Block &block)
        {
            os << "Block( " << block.type << ", Coordinate(" << block.coord->x << ", " << block.coord->y << ", " << block.coord->z << ") )";
            return os;
        }

        /// << Operator for troubleshooting.
        friend std::ostream& operator<<(std::ostream &os, const Block* block)
        {
            os << "Block( " << block->type << ", Coordinate(" << block->coord->x << ", " << block->coord->y << ", " << block->coord->z << ") )";
            return os;
        }
    };
    /**
     * A function for determining if a block is next to another block.
     *
     * @param coords:    A set containing all of the coordinates hashed together.
     *                   For the hash function see coordinate.hpp
     * @param currBlock: The current block to be checked.
     */
    void updateNeighbors(
        std::unordered_set<size_t>* coords,
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
        float* vPointer, int vIdx
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
}

#endif //OPENGLDEMO_BLOCK_HPP
