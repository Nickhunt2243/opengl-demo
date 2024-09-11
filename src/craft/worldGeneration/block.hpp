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
    struct BlockVertexData
    {
        float blockData;
        float chunkPosX;
        float chunkPosZ;
    };
    void appendAllCoordInfo(
            BlockVertexData &blockVertexData,
            Coordinate<int> blockWorldCoord,
            BlockTexture currTexture
    );
}

#endif //OPENGLDEMO_BLOCK_HPP
