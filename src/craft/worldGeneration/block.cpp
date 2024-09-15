#include "block.hpp"

namespace Craft
{
    void appendAllCoordInfo(
            NeighborInfo* visibility,
            int blockIdx,
            BlockTexture currTexture
        )
    {
        int blockData = 1;
        for (int text = 0; text < 6; text++) {
            blockData |= ((int) currTexture[text]->layer << ((text * 3) + 8)); // 3 * 6 = 18 bits | 18 total
        }
        visibility[blockIdx].sideData |= blockData;
    }
}