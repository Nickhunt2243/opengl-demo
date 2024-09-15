#version 460 core

layout(location = 0) in vec3 a_pos;

uniform float u_defaultLightLevel;
uniform int u_numChunks;
uniform int u_renderDistance;
uniform ivec2 u_minChunkCoords;

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;
const int BLOCKS_PER_CHUNK = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT;

struct BlockInformation
{
    int sideData;
};
struct ChunkInfo
{
    BlockInformation blockVisibility[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH];
};
layout (std430, binding = 0) buffer blockInformationBuffer
{
    ChunkInfo chunkInfo[];
};
int findChunkIdx(int coord)
{
    if (coord + u_renderDistance < 0)
    {
        return ((((((coord + u_renderDistance) * -1) % u_numChunks) * -1) + u_numChunks) % u_numChunks);
    }
    else
    {
        return (((coord + u_renderDistance) % u_numChunks) + u_numChunks) % u_numChunks;
    }
}
// Breaking the instance id down into the blocks coord info.
int index = gl_InstanceID + gl_BaseInstance;

int chunkNum = index / BLOCKS_PER_CHUNK;
int blockIdx = index % BLOCKS_PER_CHUNK;

ivec2 chunkNormalized = ivec2(chunkNum / u_numChunks, chunkNum % u_numChunks);
ivec2 chunk = chunkNormalized + u_minChunkCoords;

int chunkIdxX = findChunkIdx(chunk.x);
int chunkIdxZ = findChunkIdx(chunk.y);
int chunkIdx = (chunkIdxX * u_numChunks) + chunkIdxZ;

// Derive block coordinates from blockIdx
int chunkRelY = blockIdx / (CHUNK_WIDTH * CHUNK_WIDTH);
int blockRem = blockIdx % (CHUNK_WIDTH * CHUNK_WIDTH);
int chunkRelZ = (blockRem / CHUNK_WIDTH);
int chunkRelX = (blockRem % CHUNK_WIDTH);

// Aggregating data needed
ivec2 chunkPos = chunk;
ivec3 blockChunkRelPos = ivec3(chunkRelX, chunkRelY, chunkRelZ);
ivec3 worldCoords = ivec3((CHUNK_WIDTH * chunkPos.x) + chunkRelX, chunkRelY, (CHUNK_WIDTH * chunkPos.y) + chunkRelZ);
BlockInformation info = chunkInfo[chunkIdx].blockVisibility[blockIdx];

int yMaxTexture = (info.sideData >> 8) & 7;
int yMinTexture = (info.sideData >> 11) & 7;
int xMaxTexture = (info.sideData >> 14) & 7;
int xMinTexture = (info.sideData >> 17) & 7;
int zMaxTexture = (info.sideData >> 20) & 7;
int zMinTexture = (info.sideData >> 23) & 7;

out ivec3 a_blockPos;
out ivec2 a_chunkPos;
out flat float a_colorScalar;
out int a_yMaxTexture;
out int a_yMinTexture;
out int a_xMaxTexture;
out int a_xMinTexture;
out int a_zMaxTexture;
out int a_zMinTexture;
out int a_chunkIdx;
out int a_blockIdx;
out int id;
out int a_chunkNum;
out int drawBlock;
out flat int a_blockVisibility;

void main()
{
    // Transformed position data
    gl_Position = vec4(worldCoords, 1.0);
    // Game Chunk position Data
    a_blockPos = blockChunkRelPos;
    a_chunkPos = chunk;
    // Light Data
    float lightLevel = clamp(u_defaultLightLevel, 0, 15);
    a_colorScalar = (0.6 * lightLevel / 15) + 0.4;
    // Texture Data
    a_yMaxTexture = yMaxTexture;
    a_yMinTexture = yMinTexture;
    a_xMaxTexture = xMaxTexture;
    a_xMinTexture = xMinTexture;
    a_zMaxTexture = zMaxTexture;
    a_zMinTexture = zMinTexture;
    // Block Neighbor Info
    a_blockVisibility = info.sideData;
}
