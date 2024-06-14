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
//        if (coordX == 1 && coordY == 1 && coordZ == 1 && u == 1 && v == 0 && normalType == 0) {
//            std::cout << "CoordX Data: " << std::bitset<32>(coordX).to_string() << std::endl;
//            std::cout << "CoordZ Data: " << std::bitset<32>(coordZ).to_string() << std::endl;
//            std::cout << "CoordY Data: " << std::bitset<32>(coordY).to_string() << std::endl;
//            std::cout << "Textur Data: " << std::bitset<32>((int) currTexture->layer).to_string() << std::endl;
//            std::cout << "U Valu Data: " << std::bitset<32>(u).to_string() << std::endl;
//            std::cout << "V Valu Data: " << std::bitset<32>(v).to_string() << std::endl;
//            std::cout << "Normal Data: " << std::bitset<32>(normalType).to_string() << std::endl;
//
//
//
//        }
        int blockData = 0;
        blockData = blockData | (coordX);                         // 5 bits
        blockData = blockData | (coordZ << 5);                    // 5 bits | 10 total
        blockData = blockData | (coordY << 10);                   // 8 bits | 18 total
        blockData = blockData | ((int) currTexture->layer << 18); // 3 bits | 21 total
        blockData = blockData | (u << 21);                        // 1 bit  | 22 total
        blockData = blockData | (v << 22);                        // 1 bit  | 23 total
        blockData = blockData | (normalType << 23);               // 3 bit  | 26 total [0, 5] values

//        if (coordX == 6 && coordY == 6 && coordZ == 6 && u == 1 && v == 0 && normalType == 0) {
//        std::cout << "Block int : " << blockData <<  ",: " << std::bitset<32>(blockData).to_string() << std::endl;
//        std::cout << "Block Data: " << std::bitset<32>(blockData).to_string() << std::endl;

//        int x = blockData & 31;                         // 5 bits
//        int z = (blockData >> 5) & 31;                  // 5 bits
//        int y = (blockData >> 10) & 255;                 // 8 bits
//        int a_tex = (blockData >> 18) & 7;   // 2 bits
//        int newU = (blockData >> 21) & 1;                   // 1 bits
//        int newV = (blockData >> 22) & 1;               // 1 bits
//        int newNormalType = (blockData >> 23) & 7; // 3 bits
//        if (
//                coordX != x ||
//                        coordZ != z ||
//                        coordY != y ||
//                        currTexture->layer != a_tex ||
//                        u != newU ||
//                        v != newV ||
//                        normalType != newNormalType
//                ) {
//                std::cout << "CoordX Data Expected: " << coordX << ", actual" << x << std::endl;
//                std::cout << "CoordZ Data Expected: " << coordZ << ", actual" << z << std::endl;
//                std::cout << "CoordY Data Expected: " << coordY << ", actual" << y << std::endl;
//                std::cout << "Textur Data Expected: " << currTexture->layer << ", actual" << a_tex << std::endl;
//                std::cout << "U Valu Data Expected: " << u << ", actual" << newU << std::endl;
//                std::cout << "V Valu Data Expected: " << v << ", actual" << newV << std::endl;
//                std::cout << "Normal Data Expected: " << normalType << ", actual" << newNormalType << std::endl;
//        }
//        }
//        std::cout << "Block Data: " << blockData << std::endl;
        int colorMapData = 0;
        colorMapData = colorMapData | ((int) currTexture->colorMapping.r);         // 4 bits
        colorMapData = colorMapData | ((int) currTexture->colorMapping.g << 8);  // 4 bits
        colorMapData = colorMapData | ((int) currTexture->colorMapping.b << 16);  // 8 bits
        colorMapData = colorMapData | ((int) currTexture->colorMapping.a << 24);       // 1 bit
        vPointer[vIdx] = blockData;
        vPointer[vIdx+1] = colorMapData;

        return vIdx + 2;
    }

    void fillVerticesBufferData(
            Block* currBlock,
            int* vPointer, int vIdx
        )
    {
        // To See normal mapping go to assets/shaders/default.vert
        textureData* currTexture;
        // Block coordinate is the at (left, bottom, front)
        int posX = (int) currBlock->chunkRelativeCoord.x + 1,
            negX = (int) currBlock->chunkRelativeCoord.x,
            posY = (int) currBlock->chunkRelativeCoord.y + 1,
            negY = (int) currBlock->chunkRelativeCoord.y,
            posZ = (int) currBlock->chunkRelativeCoord.z + 1,
            negZ = (int) currBlock->chunkRelativeCoord.z;
        // Top vertex info
        currTexture = currBlock->textures.top;
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, posZ, 1, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, posZ, 1, 1, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, negZ, 0, 0, 0, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, negZ, 0, 1, 0, currTexture );
        // Bottom vertex info
        currTexture = currBlock->textures.bottom;
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, posZ, 0, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, posZ, 1, 0, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, negZ, 0, 1, 1, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, negZ, 1, 1, 1, currTexture );
        // Front vertex info
        currTexture = currBlock->textures.front;
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, posZ, 0, 0, 2, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, posZ, 0, 1, 2, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, posZ, 1, 0, 2, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, posZ, 1, 1, 2, currTexture );
        // Right vertex info
        currTexture = currBlock->textures.right;
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, posZ, 0, 0, 3, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, posZ, 0, 1, 3, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, negZ, 1, 0, 3, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, negZ, 1, 1, 3, currTexture );
        // Back vertex info
        currTexture = currBlock->textures.back;
        vIdx = appendCoordInfo( vPointer, vIdx, posX, posY, negZ, 0, 0, 4, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, posX, negY, negZ, 0, 1, 4, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, negZ, 1, 0, 4, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, negZ, 1, 1, 4, currTexture );
        // Left vertex info
        currTexture = currBlock->textures.left;
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, negZ, 0, 0, 5, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, negY, negZ, 0, 1, 5, currTexture );
        vIdx = appendCoordInfo( vPointer, vIdx, negX, posY, posZ, 1, 0, 5, currTexture );
               appendCoordInfo( vPointer, vIdx, negX, negY, posZ, 1, 1, 5, currTexture );
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
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> blockCoords,
            int blockIdx, int x, int y, int z,
            Block* currBlock
        )
    {
        int topIdx    = blockIdx + CHUNK_WIDTH * CHUNK_WIDTH,
            bottomIdx = blockIdx - CHUNK_WIDTH * CHUNK_WIDTH,
            frontIdx  = blockIdx + CHUNK_WIDTH,
            backIdx   = blockIdx - CHUNK_WIDTH,
            rightIdx  = blockIdx + 1,
            leftIdx   = blockIdx - 1;

        // Top
        if (
                currBlock->chunkRelativeCoord.y == CHUNK_HEIGHT - 1 || !blockCoords[topIdx]
            )
        {
            currBlock->neighborInfo.top = 1;
        }
        // bottom
        if (
                currBlock->chunkRelativeCoord.y == 0 || !blockCoords[bottomIdx]
            )
        {
            currBlock->neighborInfo.bottom = 1;
        }
        // front
        if (
                z < 15 && frontIdx < CHUNK_WIDTH && !blockCoords[frontIdx]
            )
        {
            currBlock->neighborInfo.front = 1;
        }
        // right
        if (
                x < 15 && rightIdx < CHUNK_WIDTH && !blockCoords[rightIdx]
            )
        {
            currBlock->neighborInfo.right = 1;
        }
        // back
        if (
                z > 0 && backIdx > 0 && !blockCoords[backIdx]
            )
        {
            currBlock->neighborInfo.back = 1;
        }
        // left
        if (
                x > 0 && leftIdx > 0 && !blockCoords[leftIdx]
            )
        {
            currBlock->neighborInfo.left = 1;
        }
    }
    int getVerticesCount()
    {
        // 6 = num sides * 4 = number of vertices per side * 2 = ints per vertex.
        return 6 * 4 * 2;
    }
    int getElementSize(Block* currBlock)
    {
        // 6 elements per cube face
        return currBlock->neighborInfo.sum() * 6;
    }

    bool updateLeftEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> leftChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
        )
    {
        // If block is on left side (x = 0) then we need to find same z and y
        // value at the left chunk at the 15th x idx.
        int neighborInLeftChunk = (y * CHUNK_WIDTH * CHUNK_WIDTH) + (z * CHUNK_WIDTH) + 15;
        bool rerun = currBlock->neighborInfo.left == !leftChunkBlockCoords[neighborInLeftChunk];
        currBlock->neighborInfo.left = !leftChunkBlockCoords[neighborInLeftChunk];
        return rerun;
    }

    bool updateRightEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> rightChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
    )
    {
        // If block is on left side (x = 15) then we need to find same z and y
        // value at the left chunk at the 15th x idx.
        int neighborInRightChunk = (y * CHUNK_WIDTH * CHUNK_WIDTH) + (z * CHUNK_WIDTH) + 0;

        bool rerun = currBlock->neighborInfo.right == !rightChunkBlockCoords[neighborInRightChunk];
        currBlock->neighborInfo.right = !rightChunkBlockCoords[neighborInRightChunk];
        return rerun;
    }

    bool updateFrontEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> frontChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
        )
    {
        // If block is on front side (z = 15) then we need to find same z and y
        // value at the left chunk at the 15th x idx.
        int neighborInFrontChunk = (y * CHUNK_WIDTH * CHUNK_WIDTH) + (0 * CHUNK_WIDTH) + x;
        bool rerun = currBlock->neighborInfo.front == !frontChunkBlockCoords[neighborInFrontChunk];
        currBlock->neighborInfo.front = !frontChunkBlockCoords[neighborInFrontChunk];
        return rerun;
    }

    bool updateBackEdgeNeighbors(
            std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT> backChunkBlockCoords,
            int x, int y, int z,
            Block* currBlock
    )
    {
        // If block is on left side (z = 0) then we need to find same z and y
        // value at the left chunk at the 15th x idx.
        int neighborInBackChunk = (y * CHUNK_WIDTH * CHUNK_WIDTH) + (15 * CHUNK_WIDTH) + x;
        bool rerun = currBlock->neighborInfo.back == !backChunkBlockCoords[neighborInBackChunk];
        currBlock->neighborInfo.back = !backChunkBlockCoords[neighborInBackChunk];
        return rerun;
    }
}