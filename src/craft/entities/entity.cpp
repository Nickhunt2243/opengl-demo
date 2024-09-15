//
// Created by admin on 7/3/2024.
//

#include <iostream>
#include "entity.hpp"
#include "../../helpers/helpers.hpp"


namespace Craft
{
    Entity::Entity(
            Engine::Timer* timer,
            long double x, long double y, long double z,
            Coordinate2D<int> chunkPos,
            long double front, long double back, long double left, long double right,
            std::unordered_map<Coordinate2D<int>, std::unordered_map<Coordinate<int>, Block>*>* coords
    )
            : timer{timer}
            , coords{coords}
            , entityX{x}
            , entityY{y}
            , entityZ{z}
            , entityBounds{front, back, left, right}
            , originChunk{chunkPos.x, chunkPos.z}
    {}


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
                BlockInfo info = getBlockInfo({coord.x, blockY, coord.z}, originChunk);
                if (blockExists(info, coords))
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
            BlockInfo info = getBlockInfo({coord.x, blockY, coord.z}, originChunk);

            if (blockExists(info, coords))
            {
                for (Coordinate2D<long double> entityCoord: entityBoundCoords)
                {
                    // If Coordinate is within the blocks bounds, then return true.
                    if (
                            entityCoord.x >= info.block.x &&
                            entityCoord.x <= info.block.x + 1 &&
                            entityCoord.z >= info.block.z &&
                            entityCoord.z <= info.block.z + 1
                            )
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    bool isCheckingFront(
            int xDirection,
            int zDirection,
            int blockXToCheck,
            int blockZToCheck,
            long double entityX,
            long double entityZ
        )
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
        else //if (zDirection == -1) condition must be
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
        else // if (zDirection == -1) condition must be
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
        else // if (zDirection == -1) condition must be this.
        {
            return {-scalarLeftRight, -scalarFrontBack};
        }
    }
    Coordinate2D<long double> calculateCorrection(
            int blockX, int blockZ,
            long double entityX, long double entityZ,
            int xDirection, int zDirection,
            bool checkingLeft, bool checkingFront
        )
    {
        if ((checkingFront && xDirection == 1) || (!checkingFront && xDirection == -1))
        {
            long double zDiff = (long double) blockZ - entityZ;
            float offset = (float) blockX - (float) (sqrt(PLAYER_BOUND_SQRD - (zDiff * zDiff)) + entityX);
            return {offset, 0.0l};
        }
        else if ((checkingFront && xDirection == -1) || (!checkingFront && xDirection == 1))
        {
            long double zDiff = (long double) blockZ - entityZ;
            float offset = (float) blockX - (float) (-sqrt(PLAYER_BOUND_SQRD - (zDiff * zDiff)) + entityX);
            return {offset, 0.0l};
        }
        else if ((checkingFront && zDirection == 1) || (!checkingFront && zDirection == -1))
        {
            long double xDiff = (long double) blockX - entityX;
            float offset = (float) blockZ - (float) (sqrt(PLAYER_BOUND_SQRD - (xDiff * xDiff)) + entityZ);
            return {0.0l, offset};
        }
        else if ((checkingFront && zDirection == -1) || (!checkingFront && zDirection == 1))
        {
            long double xDiff = (long double) blockX - entityX;
            float offset = (float) blockZ - (float) (-sqrt(PLAYER_BOUND_SQRD - (xDiff * xDiff)) + entityZ);
            return {0.0l, offset};
        }
        return {0.0l, 0.0f};
    }
    Coordinate2D<long double> Entity::entityCollidedHeadOn(int xDirection, int zDirection) {
        bool movingInXDir = xDirection != 0;
        // starting block to check

        int playerBlockX = (int) floor(entityX);
        int playerBlockZ = (int) floor(entityZ);
        int closestVertexX = (int) round(entityX);
        int closestVertexZ = (int) round(entityZ);
        int topBlockY = (int) round(entityY) - 1;
        int bottomBlockY = topBlockY - 1;

        BlockInfo info = getBlockInfo({closestVertexX, 0, closestVertexZ}, originChunk);
        // Blocks coordinates in world coordinates.
        long double blockChunkWorldX = (info.chunk.x * 16) + info.block.x;
        long double blockChunkWorldZ = (info.chunk.z * 16) + info.block.z;
        long double entityWorldX = getWorldX();
        long double entityWorldZ = getWorldZ();

        bool checkingFront = isCheckingFront(
            xDirection,
            zDirection,
            (int) blockChunkWorldX,
            (int) blockChunkWorldZ,
            entityWorldX,
            entityWorldZ
        );
        bool checkingLeft = isCheckingLeft(
            xDirection,
            zDirection,
            (int) blockChunkWorldX,
            (int) blockChunkWorldZ,
            entityWorldX,
            entityWorldZ
        );
        int diffX = 0, diffZ = 0;

        // Find the two blocks to check based on the direction and players current block.
        if (xDirection == 1)
        {
            diffX = checkingFront ? 1 : -1;
            diffZ = checkingLeft ? -1 : 1;
        }
        else if (xDirection == -1)
        {
            diffX = !checkingFront ? 1 : -1;
            diffZ = !checkingLeft ? -1 : 1;
        }
        else if (zDirection == 1)
        {
            diffZ = checkingFront ? 1 : -1;
            diffX = checkingLeft ? 1 : -1;
        }
        else if (zDirection == -1)
        {
            diffZ = !checkingFront ? 1 : -1;
            diffX = !checkingLeft ? 1 : -1;
        }
        std::vector<Coordinate2D<int>> blocksToCheck =
                {
                        {playerBlockX + diffX, playerBlockZ},
                        {playerBlockX, playerBlockZ + diffZ},
                };

        Coordinate2D<long double> correction{0.0l, 0.0l};
        Coordinate2D<long double> playerEdge = findPlayerEdge(xDirection, zDirection, checkingFront, checkingLeft);
        Coordinate2D<long double> edgeCoordX{entityX + playerEdge.x, entityZ};
        Coordinate2D<long double> edgeCoordZ{entityX               , entityZ + playerEdge.z};
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
        for (Coordinate2D<int> block: blocksToCheck) {
            // Checking the blockToChecks vertex to see if it is within the players bounds.
            info = getBlockInfo({block.x, (int) round(entityY) - 2, block.z}, originChunk);

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
            Coordinate<int> topBlockCoord {info.block.x, topBlockY, info.block.z};
            Coordinate<int> bottomBlockCoord {info.block.x, bottomBlockY, info.block.z};
            BlockInfo topBlock {topBlockCoord, info.chunk};
            BlockInfo bottomBlock {bottomBlockCoord, info.chunk};

            if (blockExists(topBlock, coords) || blockExists(bottomBlock, coords))
            {
                // Check if x edge is within the block.
                if (
                        correction.x == 0 &&
                        edgeCoordX.x - EPSILON >= info.block.x &&
                        edgeCoordX.x + EPSILON <= info.block.x + 1 &&
                        edgeCoordX.z - EPSILON >= info.block.z &&
                        edgeCoordX.z + EPSILON <= info.block.z + 1
                        )
                {
                    correction.x -= edgeCoordX.x - (info.block.x + blockOffsetPaddingX);
                }
                else if (
                        correction.z == 0 &&
                        edgeCoordZ.x - EPSILON >= info.block.x &&
                        edgeCoordZ.x + EPSILON <= info.block.x + 1 &&
                        edgeCoordZ.z - EPSILON >= info.block.z &&
                        edgeCoordZ.z + EPSILON <= info.block.z + 1
                        )
                {
                    correction.z -= edgeCoordZ.z - (info.block.z + blockOffsetPaddingZ);
                }
            }
        }
        return correction;
    }
    Coordinate2D<long double> Entity::entityCollidedAtCorner(int xDirection, int zDirection)
    {
        bool movingInXDir = xDirection != 0;
        // starting block to check
        int vertexXToCheck = (int) round(entityX);
        int vertexZToCheck = (int) round(entityZ);
        int topBlockY = (int) round(entityY) - 1;
        int bottomBlockY = topBlockY - 1;

        // Checking the blockToChecks vertex to see if it is within the players bounds.
        BlockInfo info = getBlockInfo({vertexXToCheck, 0, vertexZToCheck}, originChunk);
        long double blockChunkWorldX = (info.chunk.x * 16) + info.block.x;
        long double blockChunkWorldZ = (info.chunk.z * 16) + info.block.z;
        bool checkingFront = isCheckingFront(
            xDirection,
            zDirection,
            (int) blockChunkWorldX,
            (int) blockChunkWorldZ,
            getWorldX(),
            getWorldZ()
        );
        bool checkingLeft = isCheckingLeft(
            xDirection,
            zDirection,
            (int) blockChunkWorldX,
            (int) blockChunkWorldZ,
            getWorldX(),
            getWorldZ()
        );

        long double blockDistance = eucDistance(blockChunkWorldX, blockChunkWorldZ, getWorldX(), getWorldZ());
        if (blockDistance >= PLAYER_BOUND) return {0.0l, 0.0l};
        // Update the blocks x and z to get the proper blocks starting coordinate
        int blockXToCheck = vertexXToCheck;
        int blockZToCheck = vertexZToCheck;

        if (
                (checkingLeft && xDirection == 1) ||
                (!checkingLeft && xDirection == -1) ||
                (!checkingFront && zDirection == 1) ||
                (checkingFront && zDirection == -1)
            )
        {
            blockZToCheck -= 1;
        }
        if (
                (!checkingFront && xDirection == 1) ||
                (checkingFront && xDirection == -1) ||
                (!checkingLeft && zDirection == 1) ||
                (checkingLeft && zDirection == -1)
            )
        {
            blockXToCheck -= 1;
        }
        info = getBlockInfo({blockXToCheck, (int) round(entityY) - 2, blockZToCheck}, originChunk);
        Coordinate<int> topBlockCoord {info.block.x, topBlockY, info.block.z};
        Coordinate<int> bottomBlockCoord {info.block.x, bottomBlockY, info.block.z};
        BlockInfo topBlock {topBlockCoord, info.chunk};
        BlockInfo bottomBlock {bottomBlockCoord, info.chunk};
        if (blockExists(topBlock, coords) || blockExists(bottomBlock, coords))
        {
            return calculateCorrection(
                    vertexXToCheck,
                    vertexZToCheck,
                    entityX,
                    entityZ,
                    xDirection,
                    zDirection,
                    checkingLeft,
                    checkingFront
                );
        }

        return {0.0l, 0.0l};
    }
}