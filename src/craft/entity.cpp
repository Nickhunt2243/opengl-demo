//
// Created by admin on 7/3/2024.
//

#include <iostream>
#include "entity.hpp"

#define PLAYER_BOUND 0.3l
#define EPSILON 1e-4
#define VERTICAL_TOLERANCE 1e-1
#define MAX_BLOCKS (CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT)
namespace Craft
{
    Entity::Entity(
            Engine::Timer* timer,
            long double x, long double y, long double z,
            Coordinate2D<int> chunkPos,
            long double front, long double back, long double left, long double right,
            std::unordered_map<Coordinate2D<int>, std::bitset<CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT>*>* coords
    )
            : timer{timer}
            , coords{coords}
            , entityX{x}
            , entityY{y}
            , entityZ{z}
            , entityBounds{front, back, left, right}
            , originChunk{chunkPos.x, chunkPos.z}
    {}

    blockInfo Entity::getBlockInfo(int blockX, int blockZ)
    {
        blockInfo info
                {
                        blockX,
                        blockZ,
                        originChunk
                };
        if (blockZ < 0)
        {
            info.blockZ = 15;
            info.chunk.z -= 1;
        }
        else if (blockZ > 15)
        {
            info.blockZ = 0;
            info.chunk.z += 1;
        }
        else if (blockX < 0)
        {
            info.blockX = 15;
            info.chunk.x -= 1;
        }
        else if (blockX > 15)
        {
            info.blockX = 0;
            info.chunk.x += 1;
        }
        return info;
    }
    bool Entity::blockBelowEntity(float angle) {
        int originBlockX = (int) round(entityX),
                originBlockZ = (int) round(entityZ);
        int blockY = (int) round(entityY) - 3;

        // If the next Y value is not within [0.1,-.5] range of the entity then we do not check collision.
        if (blockY - (5 * VERTICAL_TOLERANCE) > (entityY - 3) || blockY + VERTICAL_TOLERANCE < (entityY - 3))
        {
            // If we are falling we return false to keep falling and if we are
            // not then we return true to not start falling.
            return vertMovement != EntityVertMovementType::FALLING && vertMovement != EntityVertMovementType::FLYING;
        }
        long double rotatedX = ((cos(-angle) * (originBlockZ - entityZ)) + (sin(-angle) * (originBlockX - entityX)) + entityX);
        long double rotatedZ = ((-sin(-angle) * (originBlockZ - entityZ)) + (cos(-angle) * (originBlockX - entityX)) + entityZ);

        // Check if the entitys FR|FL|BR|BL coords are within the bounds of a block within the surrounding 3x3 blocks
        // Check if closest vertex is within the entitys bounds.
        std::vector<Coordinate2D<int>> blocksToCheck =
                {
                        {originBlockX, originBlockZ},
                        {originBlockX - 1, originBlockZ},
                        {originBlockX, originBlockZ - 1},
                        {originBlockX - 1, originBlockZ - 1},
                };
        if (
                rotatedX <= entityX + entityBounds.left &&
                rotatedX >= entityX - entityBounds.right &&
                rotatedZ <= entityZ + entityBounds.front &&
                rotatedZ >= entityZ - entityBounds.back
                )
        {
            for (Coordinate2D<int> coord: blocksToCheck)
            {
                blockInfo info = getBlockInfo(coord.x, coord.z);
                int blockIdx = (blockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;
                if (blockIdx < CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT && blockIdx >= 0 && (*coords->at(info.chunk))[blockIdx])
                {
                    return true;
                }
            }
        }

        std::vector<Coordinate2D<long double>> entityBoundCoords =
                {
                        {
                                calcRotatedX(-entityBounds.right, entityBounds.front, angle) + entityX,
                                calcRotatedZ(-entityBounds.right, entityBounds.front, angle) + entityZ
                        },
                        {
                                calcRotatedX(entityBounds.left, entityBounds.front, angle) + entityX,
                                calcRotatedZ(entityBounds.left, entityBounds.front, angle) + entityZ
                        },
                        {
                                calcRotatedX(-entityBounds.right, -entityBounds.back, angle) + entityX,
                                calcRotatedZ(-entityBounds.right, -entityBounds.back, angle) + entityZ
                        },
                        {
                                calcRotatedX(entityBounds.left, -entityBounds.back, angle) + entityX,
                                calcRotatedZ(entityBounds.left, -entityBounds.back, angle) + entityZ
                        }
                };
        for (Coordinate2D<int> coord: blocksToCheck) {
            blockInfo info = getBlockInfo(coord.x, coord.z);
            int blockIdx = (blockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;
            if (blockIdx < CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT && blockIdx >= 0 && (*coords->at(info.chunk))[blockIdx])
            {
                for (Coordinate2D<long double> entityCoord: entityBoundCoords)
                {
                    // If Coordinate is within the blocks bounds, then return true.
                    if (
                            entityCoord.x >= info.blockX &&
                            entityCoord.x <= info.blockX + 1 &&
                            entityCoord.z >= info.blockZ &&
                            entityCoord.z <= info.blockZ + 1
                            )
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    bool isCheckingFront(int xDirection, int zDirection, int blockXToCheck, int blockZToCheck, long double entityX, long double entityZ)
    {
        if (xDirection == 1) {
            return blockXToCheck > entityX;
        }
        else if (xDirection == -1)
        {
            return blockXToCheck < entityX;
        }
        else if (zDirection == 1)
        {
            return blockZToCheck > entityZ;
        }
        else if (zDirection == -1)
        {
            return blockZToCheck < entityZ;
        }
    }
    bool isCheckingLeft(int xDirection, int zDirection, int blockXToCheck, int blockZToCheck, long double entityX, long double entityZ)
    {
        if (xDirection == 1) {
            return blockZToCheck < entityZ;
        }
        else if (xDirection == -1)
        {
            return blockZToCheck > entityZ;
        }
        else if (zDirection == 1)
        {
            return blockXToCheck > entityX;
        }
        else if (zDirection == -1)
        {
            return blockXToCheck < entityX;
        }
    }
    Coordinate2D<long double> findPlayerEdge(int xDirection, int zDirection, bool isCheckingFront, bool isCheckingLeft)
    {
        // Based on facing in the +z direction as that is angle = 0.
        long double scalarLeftRight = isCheckingLeft ? PLAYER_BOUND : -PLAYER_BOUND;
        long double scalarFrontBack = isCheckingFront ? PLAYER_BOUND : -PLAYER_BOUND;

        if (xDirection == 1)
        {
            return {scalarFrontBack, -scalarLeftRight};
        }
        else if (xDirection == -1)
        {
            return {-scalarFrontBack, scalarLeftRight};
        }
        else if (zDirection == 1)
        {
            return {scalarLeftRight, scalarFrontBack};
        }
        else if (zDirection == -1)
        {
            return {-scalarLeftRight, -scalarFrontBack};
        }
    }
    Coordinate2D<long double> calculateCorrection(
            long double distanceToVertex,
            int blockX, int blockZ,
            long double entityX, long double entityZ
        )
    {
        long double xDistanceFromVertex = blockX - entityX;
        long double zDistanceFromVertex = blockZ - entityZ;
        // We find the angle from the player origin to the nearest vertex using the x and z coordinates respectively.
        long double angleXPlayerVertex = asin(xDistanceFromVertex / distanceToVertex);
        long double angleZPlayerVertex = acos(zDistanceFromVertex / distanceToVertex);
        // From that angle we can derive the
        long double xOffset = PLAYER_BOUND * sin(angleXPlayerVertex) - xDistanceFromVertex;
        long double zOffset = PLAYER_BOUND * sin(angleZPlayerVertex) - zDistanceFromVertex;
        return {-xOffset, -zOffset};
    }
    Coordinate2D<long double> Entity::entityCollidedWithBlock(int xDirection, int zDirection)
    {
        bool movingInXDir = xDirection != 0;
        // starting block to check
        int blockXToCheck = (int) round(entityX);
        int blockZToCheck = (int) round(entityZ);
        int topBlockY = (int) round(entityY) - 1;
        int bottomBlockY = topBlockY - 1;
        std::vector<Coordinate2D<int>> blocksToCheck =
        {
            {blockXToCheck, blockZToCheck},
            {blockXToCheck - 1, blockZToCheck},
            {blockXToCheck, blockZToCheck - 1},
            {blockXToCheck - 1, blockZToCheck - 1},
        };

        bool checkingFront = isCheckingFront(xDirection, zDirection, blockXToCheck, blockZToCheck, entityX, entityZ);
        bool checkingLeft = isCheckingLeft(xDirection, zDirection, blockXToCheck, blockZToCheck, entityX, entityZ);


        Coordinate2D<int> originBlock{(int) floor(entityX), (int) floor(entityZ)};
        Coordinate2D<long double> offset{0.0l, 0.0l};
        Coordinate2D<long double> playerEdge = findPlayerEdge(xDirection, zDirection, checkingFront, checkingLeft);
        Coordinate2D<long double> edgeCoordX{entityX + playerEdge.x, entityZ};
        Coordinate2D<long double> edgeCoordZ{entityX, entityZ + playerEdge.z};
        if (edgeCoordX.x > 16)
        {
            edgeCoordX.x -= 16;
        }
        else if (edgeCoordX.x < 0)
        {
            edgeCoordX.x += 16;
        }
        if (edgeCoordZ.z > 16)
        {
            edgeCoordZ.z -= 16;
        }
        else if (edgeCoordZ.z < 0)
        {
            edgeCoordZ.z += 16;
        }
        int topBlockIdx;
        int bottomBlockIdx;
        for (Coordinate2D<int> block: blocksToCheck)
        {
            // We skip the block the player is on as the player wouldn't be here if the collision is working..
            if (block == originBlock) continue;
            // Checking the blockToChecks vertex to see if it is within the players bounds.
            blockInfo info = getBlockInfo(block.x, block.z);

            topBlockIdx = (topBlockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;
            bottomBlockIdx = (bottomBlockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;

            int blockOffsetPaddingZ = (xDirection == 1 && checkingLeft) ||
                                      (xDirection == -1 && !checkingLeft) ||
                                      (zDirection == 1 && !checkingFront) ||
                                      (zDirection == -1 && checkingFront)
                                     ? 1 : 0;

            int blockOffsetPaddingX = (xDirection == 1 && !checkingFront) ||
                                      (xDirection == -1 && checkingFront) ||
                                      (zDirection == 1 && !checkingLeft) ||
                                      (zDirection == -1 && checkingLeft)
                                      ? 1 : 0;
            if (
                    topBlockIdx >= 0 && topBlockIdx < MAX_BLOCKS && (*coords->at(info.chunk))[topBlockIdx] ||
                    bottomBlockIdx >= 0 && bottomBlockIdx < MAX_BLOCKS && (*coords->at(info.chunk))[bottomBlockIdx]
                )
            {
                // Check if x edge is within the block.
                if (
                        offset.x == 0 &&
                        edgeCoordX.x - EPSILON >= info.blockX &&
                        edgeCoordX.x + EPSILON <= info.blockX + 1 &&
                        edgeCoordX.z - EPSILON >= info.blockZ &&
                        edgeCoordX.z + EPSILON <= info.blockZ + 1
                    )
                {
                    offset.x -= edgeCoordX.x - (info.blockX + blockOffsetPaddingX);
                }
                else if (
                        offset.z == 0 &&
                        edgeCoordZ.x - EPSILON >= info.blockX &&
                        edgeCoordZ.x + EPSILON <= info.blockX + 1 &&
                        edgeCoordZ.z - EPSILON >= info.blockZ &&
                        edgeCoordZ.z + EPSILON <= info.blockZ + 1
                    )
                {
                    offset.z -= edgeCoordZ.z - (info.blockZ + blockOffsetPaddingZ);
                }
            }
        }
        if (offset.x != 0.0l || offset.z != 0.0l) return offset;

        // Checking the blockToChecks vertex to see if it is within the players bounds.
        blockInfo info = getBlockInfo(blockXToCheck, blockZToCheck);

        long double blockDistance = eucDistance(entityX, entityZ, info.blockX, info.blockZ);
        if (blockDistance < PLAYER_BOUND)
        {
            for (Coordinate2D<int> block: blocksToCheck)
            {
                info = getBlockInfo(block.x, block.z);

                topBlockIdx = (topBlockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;
                bottomBlockIdx = (bottomBlockY * CHUNK_WIDTH * CHUNK_WIDTH) + (info.blockZ * CHUNK_WIDTH) + info.blockX;
                if (
                    (topBlockIdx >= 0 && topBlockIdx < MAX_BLOCKS && (*coords->at(info.chunk))[topBlockIdx]) ||
                    (bottomBlockIdx >= 0 && bottomBlockIdx < MAX_BLOCKS && (*coords->at(info.chunk))[bottomBlockIdx])
                )
                {
                    return calculateCorrection(blockDistance, blockXToCheck, blockZToCheck, entityX, entityZ);
                }
            }
        }

        return {0.0l, 0.0l};
    }
}