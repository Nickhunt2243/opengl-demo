#version 460 core

layout(location = 0) in int blockData;
layout(location = 1) in int colorMappingData;

// 4 Bytes of block Data (7 bits free. Most likely will need them for textures)
// 5 bits of x data
int x = blockData & 31;
// 5 bits of z data
int z = (blockData >> 5) & 31;
// 8 bits of y data
int y = (blockData >> 10) & 255;
// 3 bits of texture data
int a_tex = (blockData >> 18) & 7;
// 2 bits of uv data
vec2 a_uv = vec2(
    (blockData >> 21) & 1,
    (blockData >> 22) & 1
);
// 3 bits of normal data
int normalType = (blockData >> 23) & 7; // 3 bits
int lightScalar = (blockData >> 26) & 15; // 4 bits


vec4 a_colorMap = vec4(
    (colorMappingData & 255) / 255.0, // Normalize to [0, 1]
    ((colorMappingData >> 8) & 255) / 255.0,
    ((colorMappingData >> 16) & 255) / 255.0,
    ((colorMappingData >> 24) & 255) / 255.0
);

uniform vec2 u_ChunkPos;
uniform mat4 u_projT;
uniform mat4 u_viewT;
uniform float u_DefaultLightLevel;

out vec2 texCoords;
out int currTex;
out flat vec4 colorMap;
out vec3 a_blockPos;
out vec2 a_chunkPos;
out flat vec3 a_norm;
out flat float a_colorScalar;

vec4 newPos = vec4(
    x + u_ChunkPos.x,
    y,
    z + u_ChunkPos.y,
    1.0
);

vec3[] normals = {
    {0.0, 1.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, -1.0},
    {1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0},
    {-1.0, 0.0, 0.0}
};


void main()
{

    gl_Position = u_projT * u_viewT * newPos;
    a_chunkPos = vec2(x, z);
    a_blockPos = newPos.xyz;
    float lightLevel = clamp(u_DefaultLightLevel - lightScalar, 0, 15);
    a_colorScalar = (0.6 * lightLevel / 15) + 0.4;
    texCoords = a_uv;
    currTex = a_tex;
    colorMap = a_colorMap;
    a_norm = normals[normalType];
}
