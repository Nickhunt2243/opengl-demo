#version 460 core

// Future plans to reduce the vertex buffer size.
//layout(location = 0) in int blockData; // 4 Bytes (8 bits free.)
//vec4 a_pos = vec4(                     // 18 bits
//    blockData & 31,                         // 5 bits
//    (blockData >> 4) & 31,                  // 5 bits
//    (blockData >> 8) & 255,                 // 8 bits
//    1.0
//);
//float a_tex = (blockData >> 16) & 3;   // 2 bits
//vec2 a_uv = vec2(                      // 2 bites
//    (blockData >> 18) & 1,                  // 1 bits
//    (blockData >> 19) & 1                   // 1 bits
//);
//vec3 a_norm = vec3(                    // 3 bits
//    (blockData >> 20) & 1,                  // 1 bits
//    (blockData >> 21) & 1,                  // 1 bits
//    (blockData >> 22) & 1                   // 1 bits
//);
//
//layout(location = 1) in int colorMappingData; // 4 Bytes (0 bits free.)
//vec4 a_colorMap = vec4(
//    (colorMappingData) & 255, // 8 bits
//    (colorMappingData >> 8) & 255, // 8 bits
//    (colorMappingData >> 16) & 255, // 8 bits
//    (colorMappingData >> 24) & 255  // 8 bits
//);
// uniform vec2 u_ChunkPos

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in float a_tex;
layout(location = 3) in vec4 a_colorMap;

uniform mat4 u_projT;
uniform mat4 u_viewT;

out vec2 texCoords;
out flat float currTex;
//out vec2 colorMapCoords;
out flat vec4 colorMap;

void main()
{
    gl_Position = u_projT * u_viewT * vec4(a_pos, 1.0);
    texCoords = a_uv;
    currTex = a_tex;
    colorMap = a_colorMap;
}
