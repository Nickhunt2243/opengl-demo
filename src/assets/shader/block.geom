// Geometry shader example
#version 460 core
layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

// Constants
const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;

// Inputs from the vertex shader
in ivec3 a_blockPos[];
in ivec2 a_chunkPos[];
in flat float a_colorScalar[];
in int a_yMaxTexture[];
in int a_yMinTexture[];
in int a_xMaxTexture[];
in int a_xMinTexture[];
in int a_zMaxTexture[];
in int a_zMinTexture[];

// Outputs to the fragment shader

out vec2 g_texCoords;
//out ivec2 g_chunkPos;
out flat int g_currTex;
out flat vec4 g_colorMap;
out flat ivec3 g_blockPos;
out flat ivec3 g_norm;
out flat float g_colorScalar;

// Uniform from your application
uniform ivec3 u_CameraNorm;
uniform int u_NumChunks;
uniform int u_RenderDistance;
uniform mat4 u_projT;
uniform mat4 u_viewT;

void setOutputs(
        vec4 position,
        vec2 texCoords,
        int currTex,
        ivec3 norm
    )
{
    gl_Position = position;
    g_texCoords = texCoords;
    g_currTex = currTex;
    g_norm = norm;
    // Defaults from input
    g_blockPos = a_blockPos[0];
    g_colorScalar = a_colorScalar[0];
    EmitVertex();
}

struct blockV{
    int data;
//    int chunkPosX;
//    int chunkPosZ;
//    int chunkIdx;
//    float blockX;
//    float blockZ;
//    float blockY;
////    float relBlockX;
////    float relBlockZ;
////    float relBlockY;
//    int chunkIdxX;
//    int chunkIdxZ;
//    int u_RenderDistance;
//    int u_NumChunks;
};

struct ChunkInfo
{
    blockV blockVisibility[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH];
};
layout (std430, binding = 0) buffer blockInformationBuffer
{
    ChunkInfo chunkInfo[];
};

int findChunkIdx(int coord)
{
    if ((coord + u_RenderDistance) < 0)
    {
        return ((((((coord + u_RenderDistance) * -1) % u_NumChunks) * -1) + u_NumChunks) % u_NumChunks);
    }
    else
    {
        return (((coord + u_RenderDistance) % u_NumChunks) + u_NumChunks) % u_NumChunks;
    }
}

void main()
{

    // Compare the normal from the first vertex with the camera normal
    int chunkIdxX = findChunkIdx(a_chunkPos[0].x);
    int chunkIdxZ = findChunkIdx(a_chunkPos[0].y);
//
//    int chunkIdxX = (((a_chunkPos[0].x + u_RenderDistance) % u_NumChunks) + u_NumChunks) % u_NumChunks;
//    int chunkIdxZ = (((a_chunkPos[0].y + u_RenderDistance) % u_NumChunks) + u_NumChunks) % u_NumChunks;
    int chunkIdx = (chunkIdxX * u_NumChunks) + chunkIdxZ;
    int blockIdx = (a_blockPos[0].y * CHUNK_WIDTH * CHUNK_WIDTH) + (a_blockPos[0].z * CHUNK_WIDTH) + a_blockPos[0].x;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].chunkPosX = a_chunkPos[0].x;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].chunkPosZ = a_chunkPos[0].y;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].chunkIdxX = chunkIdxX;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].chunkIdxZ = chunkIdxZ;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].u_RenderDistance = u_RenderDistance;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].u_NumChunks = u_NumChunks;
//
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].chunkIdx = chunkIdx;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].blockX = gl_in[0].gl_Position.x;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].blockZ = gl_in[0].gl_Position.z;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].blockY = gl_in[0].gl_Position.y;
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].relBlockX = (a_chunkPos[0].x + u_RenderDistance); // -2 + 1
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].relBlockZ = ((a_chunkPos[0].x + u_RenderDistance) % u_NumChunks); // -1 % 3
//    chunkInfo[chunkIdx].blockVisibility[blockIdx].relBlockY = ((a_chunkPos[0].x + u_RenderDistance) % u_NumChunks) + u_NumChunks;

    int blockVisibility = chunkInfo[chunkIdx].blockVisibility[blockIdx].data;
    ivec3 norm;
    mat4 transMatrix = u_projT * u_viewT;

    vec4 bottomLeftFront = transMatrix * gl_in[0].gl_Position;                       // negX, negY, negZ
    vec4 bottomLeftBack = transMatrix * (gl_in[0].gl_Position + vec4(1, 0, 0, 0));   // posX, negY, negZ
    vec4 bottomRightFront = transMatrix * (gl_in[0].gl_Position + vec4(0, 0, 1, 0)); // negX, negY, posZ
    vec4 bottomRightBack = transMatrix * (gl_in[0].gl_Position + vec4(1, 0, 1, 0));  // posX, negY, posZ

    vec4 topLeftFront = transMatrix * (gl_in[0].gl_Position + vec4(0, 1, 0, 0));     // negX, posY, negZ
    vec4 topLeftBack = transMatrix * (gl_in[0].gl_Position + vec4(1, 1, 0, 0));      // posX, posY, negZ
    vec4 topRightFront = transMatrix * (gl_in[0].gl_Position + vec4(0, 1, 1, 0));    // negX, posY, posZ
    vec4 topRightBack = transMatrix * (gl_in[0].gl_Position + vec4(1, 1, 1, 0));     // posX, posY, posZ

    vec2 uv00 = vec2(0, 0);
    vec2 uv01 = vec2(0, 1);
    vec2 uv10 = vec2(1, 0);
    vec2 uv11 = vec2(1, 1);

    // Top vertex info
    norm = ivec3(0, 1, 0);
    if ((blockVisibility & 2) == 2)
    {
        setOutputs( topRightFront, uv10, a_yMaxTexture[0], norm);
        setOutputs( topRightBack, uv11, a_yMaxTexture[0], norm);
        setOutputs( topLeftFront, uv00, a_yMaxTexture[0], norm);
        setOutputs( topLeftBack, uv01, a_yMaxTexture[0], norm);
        EndPrimitive();
    }
    // Bottom vertex info
    norm = ivec3(0, -1, 0);
    if ((blockVisibility & 4) == 4)
    {
        setOutputs(bottomRightBack, uv00, a_yMinTexture[0], norm);
        setOutputs(bottomRightFront, uv10, a_yMinTexture[0], norm);
        setOutputs(bottomLeftBack, uv01, a_yMinTexture[0], norm);
        setOutputs(bottomLeftFront, uv11, a_yMinTexture[0], norm);
        EndPrimitive();
    }
    // X_MAX (old back) vertex info
    norm = ivec3(1, 0, 0);
    if ((blockVisibility & 8) == 8 && norm != u_CameraNorm)
    {
        setOutputs(topRightBack, uv00, a_xMaxTexture[0], norm);
        setOutputs(bottomRightBack, uv01, a_xMaxTexture[0], norm);
        setOutputs(topLeftBack, uv10, a_xMaxTexture[0], norm);
        setOutputs(bottomLeftBack, uv11, a_xMaxTexture[0], norm);
        EndPrimitive();
    }
    // X_MIN (old front) vertex info
    norm = ivec3(-1, 0, 0);
    if ((blockVisibility & 16) == 16 && norm != u_CameraNorm)
    {
        setOutputs(topLeftFront, uv00, a_xMinTexture[0], norm);
        setOutputs(bottomLeftFront, uv01, a_xMinTexture[0], norm);
        setOutputs(topRightFront, uv10, a_xMinTexture[0], norm);
        setOutputs(bottomRightFront, uv11, a_xMinTexture[0], norm);
        EndPrimitive();
    }
    // Z_MAX (old right) vertex info
    norm = ivec3(0, 0, 1);
    if ((blockVisibility & 32) == 32 && norm != u_CameraNorm)
    {
        setOutputs(topRightFront, uv00, a_zMaxTexture[0], norm);
        setOutputs(bottomRightFront, uv01, a_zMaxTexture[0], norm);
        setOutputs(topRightBack, uv10, a_zMaxTexture[0], norm);
        setOutputs(bottomRightBack, uv11, a_zMaxTexture[0], norm);
        EndPrimitive();
    }
    // Left vertex info
    norm = ivec3(0, 0, -1);
    if ((blockVisibility & 64) == 64 && norm != u_CameraNorm)
    {
        setOutputs(topLeftBack, uv00, a_zMinTexture[0], norm);
        setOutputs(bottomLeftBack, uv01, a_zMinTexture[0], norm);
        setOutputs(topLeftFront, uv10, a_zMinTexture[0], norm);
        setOutputs(bottomLeftFront, uv11, a_zMinTexture[0], norm);
        EndPrimitive();
    }
}