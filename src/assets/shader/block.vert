#version 460 core

layout(location = 0) in float blockData;
layout(location = 1) in float blockX;
layout(location = 2) in float blockZ;

uniform float u_DefaultLightLevel;
uniform int u_NumChunks;
uniform int u_RenderDistance;

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;
//
//int worldWidth = CHUNK_WIDTH * u_NumChunks;

// Getting blockX by dividint the instance ID by number of blocks in a slice of the worlds z and y axis
//float blockY = gl_InstanceID / (worldWidth * worldWidth);
//float blockZ = (blockY - floor(blockY)) * worldWidth;
//float blockX = (blockZ - floor(blockZ)) * worldWidth;

int blockDataInt = int(blockData);

// 8 bits of y data
float blockY = blockDataInt & 255;
// 3 bits of texture data with 6 diff textures
int yMaxTexture = (blockDataInt >> 8) & 7;
int yMinTexture = (blockDataInt >> 11) & 7;
int xMaxTexture = (blockDataInt >> 14) & 7;
int xMinTexture = (blockDataInt >> 17) & 7;
int zMaxTexture = (blockDataInt >> 20) & 7;
int zMinTexture = (blockDataInt >> 23) & 7;

ivec2 chunkPos = ivec2(int(floor(blockX / 16.0)), int(floor(blockZ / 16.0)));
ivec3 blockChunkRelPos = ivec3(int(blockX) - (chunkPos.x * 16), blockY, int(blockZ) - (chunkPos.y * 16));

out ivec3 a_blockPos;
out ivec2 a_chunkPos;
out flat float a_colorScalar;
out int a_yMaxTexture;
out int a_yMinTexture;
out int a_xMaxTexture;
out int a_xMinTexture;
out int a_zMaxTexture;
out int a_zMinTexture;

void main()
{
    // Transformed position data
    gl_Position = vec4(blockX, blockY, blockZ, 1.0);
    // Game Chunk position Data
    a_blockPos = blockChunkRelPos;
    a_chunkPos = chunkPos;
    // Light Data
    float lightLevel = clamp(u_DefaultLightLevel, 0, 15);
    a_colorScalar = (0.6 * lightLevel / 15) + 0.4;
    // Texture Data
    a_yMaxTexture = yMaxTexture;
    a_yMinTexture = yMinTexture;
    a_xMaxTexture = xMaxTexture;
    a_xMinTexture = xMinTexture;
    a_zMaxTexture = zMaxTexture;
    a_zMinTexture = zMinTexture;
}
