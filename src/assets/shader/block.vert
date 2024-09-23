#version 460 core

layout(location = 0) in ivec3 a_pos;
layout(location = 1) in ivec3 a_norm;
layout(location = 2) in ivec2 a_uv;
layout(location = 3) in int a_textureMask;
layout(location = 4) in int a_visibilityMask;
layout(location = 5) in ivec2 a_ambientInfo;

uniform float u_defaultLightLevel;
uniform int u_numChunks;
uniform ivec2 u_minChunkCoords;
uniform mat4 u_projT;
uniform mat4 u_viewT;

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;
const int BLOCKS_IN_CHUNK = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT;

struct BlockInformation
{
    int sideData;
    int lighting[3];
};
layout (std430, binding = 0) buffer blockInformationBuffer
{
    BlockInformation blockInfo[];
};
layout (std430, binding = 1) buffer chunkPosInformationBuffer
{
    ivec2 chunkInfo[];
};
layout (std430, binding = 2) buffer idxInformationBuffer
{
    int idxs[];
};

int index = idxs[gl_InstanceID + gl_BaseInstance];

int chunkIdx = index / BLOCKS_IN_CHUNK;
int blockIdx = index % BLOCKS_IN_CHUNK;
int chunkRelY = blockIdx / (CHUNK_WIDTH * CHUNK_WIDTH);
int blockRem = blockIdx % (CHUNK_WIDTH * CHUNK_WIDTH);
int chunkRelZ = (blockRem / CHUNK_WIDTH);
int chunkRelX = (blockRem % CHUNK_WIDTH);

BlockInformation info = blockInfo[index];
ivec2 chunkPos = chunkInfo[chunkIdx];
ivec3 blockChunkRelPos = ivec3(chunkRelX, chunkRelY, chunkRelZ);
ivec3 worldCoords = ivec3(
    (chunkPos.x * CHUNK_WIDTH) + blockChunkRelPos.x,
    blockChunkRelPos.y,
    (chunkPos.y * CHUNK_WIDTH) + blockChunkRelPos.z
);

int textureInfo = (info.sideData >> a_textureMask) & 7;

out flat ivec3 v_blockPos;
out flat ivec2 v_chunkPos;
out flat float v_colorScalar;
out flat ivec3 v_norm;
out vec2 v_uv;
out flat int v_textureInfo;
out float v_ambientValue;
void main()
{
    // Transformed position data
    gl_Position = u_projT * u_viewT * vec4(worldCoords + a_pos, 1.0);
    // Game Chunk position Data
    v_blockPos = worldCoords;
    v_chunkPos = chunkPos;
    // Light Data
    float lightLevel = clamp(u_defaultLightLevel, 0, 15);
    v_colorScalar = (0.6 * lightLevel / 15) + 0.4;
    // Texture Data
    v_norm = a_norm;
    v_uv = a_uv;
    v_textureInfo = textureInfo;
    // Block Neighbor Info
    // Check whether the block exists and if the side has a neighbor
    v_ambientValue = (info.lighting[a_ambientInfo.x] >> a_ambientInfo.y) & 3;
}
