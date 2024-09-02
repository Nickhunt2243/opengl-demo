//
// Created by admin on 6/6/2024.
//

#include <unordered_set>
#include <iostream>

#include "block.hpp"

namespace Craft
{
    int appendCoordInfo(
            int* vPointer, int vIdx,
            int coordX, int coordY, int coordZ,
            int u, int v,
            int normalType,
            textureData* currTexture
        )
    {
        int blockData = 0;
        blockData = blockData | (coordX);                         // 5 bits
        blockData = blockData | (coordZ << 5);                    // 5 bits | 10 total
        blockData = blockData | (coordY << 10);                   // 8 bits | 18 total
        blockData = blockData | ((int) currTexture->layer << 18); // 3 bits | 21 total
        blockData = blockData | (u << 21);                        // 1 bit  | 22 total
        blockData = blockData | (v << 22);                        // 1 bit  | 23 total
        blockData = blockData | (normalType << 23);               // 3 bit  | 26 total [0, 5] values

        int colorMapData = 0;
        colorMapData = colorMapData | ((int) currTexture->colorMapping.r);        // 8 bits
        colorMapData = colorMapData | ((int) currTexture->colorMapping.g << 8);   // 8 bits
        colorMapData = colorMapData | ((int) currTexture->colorMapping.b << 16);  // 8 bits
        colorMapData = colorMapData | ((int) currTexture->colorMapping.a << 24);  // 8 bits
        vPointer[vIdx] = blockData;
        vPointer[vIdx+1] = colorMapData;

        return vIdx + 2;
    }

    void fillVerticesBufferData(
            Coordinate<int> chunkRelativeCoord,
            Block currBlock,
            int* vPointer, int vIdx,
            blockTexture textures
        )
    {
        // To See normal mapping go to assets/shaders/default.vert
        textureData* currTexture;
        // Block coordinate is the at (left, bottom, front)
        int posX = (int) chunkRelativeCoord.x + 1,
            negX = (int) chunkRelativeCoord.x,
            posY = (int) chunkRelativeCoord.y + 1,
            negY = (int) chunkRelativeCoord.y,
            posZ = (int) chunkRelativeCoord.z + 1,
            negZ = (int) chunkRelativeCoord.z;

        // Top vertex info
        currTexture = textures.top;
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, posZ, 1, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, posZ, 1, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, negZ, 0, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, negZ, 0, 1, 0, currTexture );
        // Bottom vertex info
        currTexture = textures.bottom;
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, posZ, 0, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, posZ, 1, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, negZ, 0, 1, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, negZ, 1, 1, 1, currTexture );
        // Front vertex info

        currTexture = textures.front;
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, negZ, 0, 0, 5, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, negZ, 0, 1, 5, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, posZ, 1, 0, 5, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, posZ, 1, 1, 5, currTexture );
        // Right vertex info
        currTexture = textures.right;
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, posZ, 0, 0, 4, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, posZ, 0, 1, 4, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, posZ, 1, 0, 4, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, posZ, 1, 1, 4, currTexture );
        // Back vertex info
        currTexture = textures.back;
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, posZ, 0, 0, 3, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, posZ, 0, 1, 3, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, negZ, 1, 0, 3, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, negZ, 1, 1, 3, currTexture );
        // Left vertex info
        currTexture = textures.left;
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, negZ, 0, 0, 2, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, negZ, 0, 1, 2, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, negZ, 1, 0, 2, currTexture );
        appendCoordInfo( vPointer, vIdx, negX, negY, negZ, 1, 1, 2, currTexture );
    }
    void appendInstance(uint32_t* ePointer, int eIdx, int currIndex)
    {
        ePointer[eIdx] = currIndex;
        ePointer[eIdx + 1] = currIndex+1;
        ePointer[eIdx + 2] = currIndex+2;
        ePointer[eIdx + 3] = currIndex+3;
        ePointer[eIdx + 4] = currIndex+2;
        ePointer[eIdx + 5] = currIndex+1;
    }
    void fillElementBufferData(
            uint32_t* ePointer, int eIdx,
            int currIndex,
            NeighborsInfo neighborInfo
        )
    {
        if (neighborInfo.y_max() == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (neighborInfo.y_min() == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (neighborInfo.x_min() == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (neighborInfo.z_max() == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (neighborInfo.x_max() == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (neighborInfo.z_min() == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
        }
    }
    int getVerticesCount()
    {
        // 6 = num sides * 4 = number of vertices per side * 2 = ints per vertex.
        return 6 * 4 * 2;
    }
}