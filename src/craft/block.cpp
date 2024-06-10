//
// Created by admin on 6/6/2024.
//

#include <unordered_set>
#include <iostream>

#include "block.hpp"

namespace Craft
{
    int appendCoordInfo(
            float* vPointer, int vIdx,
            float coordX, float coordY, float coordZ,
            float u, float v,
            textureData* currTexture
        )
    {
        vPointer[vIdx] = coordX; vPointer[vIdx + 1] = coordY; vPointer[vIdx + 2] = coordZ;
        vPointer[vIdx + 3] = u; vPointer[vIdx + 4] = v;
        vPointer[vIdx + 5] = static_cast<float>(currTexture->layer);
        vPointer[vIdx + 6] = currTexture->colorMapping.r;
        vPointer[vIdx + 7] = currTexture->colorMapping.g;
        vPointer[vIdx + 8] = currTexture->colorMapping.b;
        vPointer[vIdx + 9] = currTexture->colorMapping.a;

        return vIdx + 10;
    }

    void fillVerticesBufferData(
            Block* currBlock,
            float* vPointer, int vIdx
        )
    {
        textureData* currTexture;
        float posDeltaX = currBlock->coord->x + CUBE_DELTA,
                negDeltaX = currBlock->coord->x - CUBE_DELTA,
                posDeltaY = currBlock->coord->y + CUBE_DELTA,
                negDeltaY = currBlock->coord->y - CUBE_DELTA,
                posDeltaZ = currBlock->coord->z + CUBE_DELTA,
                negDeltaZ = currBlock->coord->z - CUBE_DELTA;

        // Top vertex info
        currTexture = currBlock->textures.top;
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, posDeltaY, posDeltaZ, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, posDeltaY, posDeltaZ, 1, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, posDeltaY, negDeltaZ, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, posDeltaY, negDeltaZ, 0, 1, currTexture );
        // Bottom vertex info
        currTexture = currBlock->textures.bottom;
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, negDeltaY, posDeltaZ, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, negDeltaY, posDeltaZ, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, negDeltaY, negDeltaZ, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, negDeltaY, negDeltaZ, 1, 1, currTexture );
        // Front vertex info
        currTexture = currBlock->textures.front;
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, posDeltaY, posDeltaZ, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, negDeltaY, posDeltaZ, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, posDeltaY, posDeltaZ, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, negDeltaY, posDeltaZ, 1, 1, currTexture );
        // Right vertex info
        currTexture = currBlock->textures.right;
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, posDeltaY, posDeltaZ, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, negDeltaY, posDeltaZ, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, posDeltaY, negDeltaZ, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, negDeltaY, negDeltaZ, 1, 1, currTexture );
        // Back vertex info
        currTexture = currBlock->textures.back;
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, posDeltaY, negDeltaZ, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posDeltaX, negDeltaY, negDeltaZ, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, posDeltaY, negDeltaZ, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, negDeltaY, negDeltaZ, 1, 1, currTexture );
        // Left vertex info
        currTexture = currBlock->textures.left;
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, posDeltaY, negDeltaZ, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, negDeltaY, negDeltaZ, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negDeltaX, posDeltaY, posDeltaZ, 1, 0, currTexture );
        appendCoordInfo( vPointer, vIdx, negDeltaX, negDeltaY, posDeltaZ, 1, 1, currTexture );
    }
    void appendInstance(unsigned int* ePointer, int eIdx, int currIndex)
    {
        ePointer[eIdx] = currIndex;
        ePointer[eIdx + 1] = currIndex+1;
        ePointer[eIdx + 2] = currIndex+2;
        ePointer[eIdx + 3] = currIndex+3;
        ePointer[eIdx + 4] = currIndex+2;
        ePointer[eIdx + 5] = currIndex+1;
    }
    void fillElementBufferData(
            Block* currBlock,
            unsigned int* ePointer, int eIdx,
            int currIndex
        )
    {
        if (currBlock->neighborInfo.top == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (currBlock->neighborInfo.bottom == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (currBlock->neighborInfo.front == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (currBlock->neighborInfo.right == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (currBlock->neighborInfo.back == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
            eIdx += 6;
        }
        currIndex+=4;
        if (currBlock->neighborInfo.left == 1)
        {
            appendInstance(ePointer, eIdx, currIndex);
        }
    }
    void updateNeighbors(
            std::unordered_set<size_t>* coords,
            Block* currBlock
        )
    {
        size_t topCoordHash = currBlock->coord->add(0.0f, 1.0f, 0.0f);
        size_t bottomCoordHash = currBlock->coord->add(0.0f, -1.0f, 0.0f);
        size_t frontCoordHash = currBlock->coord->add(0.0f, 0.0f, 1.0f);
        size_t rightCoordHash = currBlock->coord->add(1.0f, 0.0f, 0.0f);
        size_t backCoordHash = currBlock->coord->add(0.0f, 0.0f, -1.0f);
        size_t leftCoordHash = currBlock->coord->add(-1.0f, 0.0f, 0.0f);

        auto endIter = coords->end();
        // Top
        if (coords->find(topCoordHash) == endIter)
        {
            currBlock->neighborInfo.top = 1;
        }
        else
        {
            currBlock->neighborInfo.top = 0;
        }
        // bottom
        if (coords->find(bottomCoordHash) == endIter)
        {
            currBlock->neighborInfo.bottom = 1;
        }
        else
        {
            currBlock->neighborInfo.bottom = 0;
        }
        // front
        if (coords->find(frontCoordHash) == endIter)
        {
            currBlock->neighborInfo.front = 1;
        }
        else
        {
            currBlock->neighborInfo.front = 0;
        }
        // right
        if (coords->find(rightCoordHash) == endIter)
        {
            currBlock->neighborInfo.right = 1;
        }
        else
        {
            currBlock->neighborInfo.right = 0;
        }
        // back
        if (coords->find(backCoordHash) == endIter)
        {
            currBlock->neighborInfo.back = 1;
        }
        else
        {
            currBlock->neighborInfo.back = 0;
        }
        // left
        if (coords->find(leftCoordHash) == endIter)
        {
            currBlock->neighborInfo.left = 1;
        }
        else
        {
            currBlock->neighborInfo.left = 0;
        }
    }
    int getVerticesCount()
    {
        // neighborsInfo.sum = number of sides to render and 4 = number of vertices per side * 6 floats per vertex
        return 6 * 4 * 10;
    }
    int getElementSize(Block* currBlock)
    {
        // 6 elements per cube face
        return currBlock->neighborInfo.sum() * 6;
    }
}