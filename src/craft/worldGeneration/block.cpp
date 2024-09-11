#include "block.hpp"

namespace Craft
{
    void appendAllCoordInfo(
            BlockVertexData &blockVertexData,
            Coordinate<int> blockWorldCoord,
            BlockTexture currTexture
        ) {
        int blockData = 0;
        blockData |= (blockWorldCoord.y);
        for (int text = 0; text < 6; text++) {
            blockData |= ((int) currTexture[text]->layer << ((text * 3) + 8)); // 3 * 6 = 18 bits | 18 total
        }
        blockVertexData.blockData = (float) blockData;
        blockVertexData.chunkPosX = (float) blockWorldCoord.x;
        blockVertexData.chunkPosZ = (float) blockWorldCoord.z;
    }
}