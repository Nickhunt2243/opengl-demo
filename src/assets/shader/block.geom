// Geometry shader example
#version 460 core
layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

// Constants
const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;
const int BLOCK_EXISTS = 1;
const int Y_MAX_VISIBLE = 2;
const int Y_MIN_VISIBLE = 4;
const int X_MAX_VISIBLE = 8;
const int X_MIN_VISIBLE = 16;
const int Z_MAX_VISIBLE = 32;
const int Z_MIN_VISIBLE = 64;

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
in flat int a_blockVisibility[];

// Outputs to the fragment shader
out vec2 g_texCoords;
out flat int g_currTex;
out flat vec4 g_colorMap;
out flat ivec3 g_blockPos;
out flat ivec3 g_norm;
out flat float g_colorScalar;

// Uniform from your application
uniform ivec3 u_CameraNorm;
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

void main()
{
    if ((a_blockVisibility[0] & BLOCK_EXISTS) != 1)
    {
        return;
    }
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
    if ((a_blockVisibility[0] & Y_MAX_VISIBLE) == Y_MAX_VISIBLE)
    {
        norm = ivec3(0, 1, 0);
        setOutputs( topRightFront, uv10, a_yMaxTexture[0], norm);
        setOutputs( topRightBack, uv11, a_yMaxTexture[0], norm);
        setOutputs( topLeftFront, uv00, a_yMaxTexture[0], norm);
        setOutputs( topLeftBack, uv01, a_yMaxTexture[0], norm);
        EndPrimitive();
    }
    // Bottom vertex info
    if ((a_blockVisibility[0] & Y_MIN_VISIBLE) == Y_MIN_VISIBLE)
    {
        norm = ivec3(0, -1, 0);
        setOutputs(bottomRightBack, uv00, a_yMinTexture[0], norm);
        setOutputs(bottomRightFront, uv10, a_yMinTexture[0], norm);
        setOutputs(bottomLeftBack, uv01, a_yMinTexture[0], norm);
        setOutputs(bottomLeftFront, uv11, a_yMinTexture[0], norm);
        EndPrimitive();
    }
    // X_MAX (old back) vertex info
    if ((a_blockVisibility[0] & X_MAX_VISIBLE) == X_MAX_VISIBLE && norm != u_CameraNorm)
    {
        norm = ivec3(1, 0, 0);
        setOutputs(topRightBack, uv00, a_xMaxTexture[0], norm);
        setOutputs(bottomRightBack, uv01, a_xMaxTexture[0], norm);
        setOutputs(topLeftBack, uv10, a_xMaxTexture[0], norm);
        setOutputs(bottomLeftBack, uv11, a_xMaxTexture[0], norm);
        EndPrimitive();
    }
    // X_MIN (old front) vertex info
    if ((a_blockVisibility[0] & X_MIN_VISIBLE) == X_MIN_VISIBLE && norm != u_CameraNorm)
    {
        norm = ivec3(-1, 0, 0);
        setOutputs(topLeftFront, uv00, a_xMinTexture[0], norm);
        setOutputs(bottomLeftFront, uv01, a_xMinTexture[0], norm);
        setOutputs(topRightFront, uv10, a_xMinTexture[0], norm);
        setOutputs(bottomRightFront, uv11, a_xMinTexture[0], norm);
        EndPrimitive();
    }
    // Z_MAX (old right) vertex info
    if ((a_blockVisibility[0] & Z_MAX_VISIBLE) == Z_MAX_VISIBLE && norm != u_CameraNorm)
    {
        norm = ivec3(0, 0, 1);
        setOutputs(topRightFront, uv00, a_zMaxTexture[0], norm);
        setOutputs(bottomRightFront, uv01, a_zMaxTexture[0], norm);
        setOutputs(topRightBack, uv10, a_zMaxTexture[0], norm);
        setOutputs(bottomRightBack, uv11, a_zMaxTexture[0], norm);
        EndPrimitive();
    }
    // Left vertex info
    if ((a_blockVisibility[0] & Z_MIN_VISIBLE) == Z_MIN_VISIBLE && norm != u_CameraNorm)
    {
        norm = ivec3(0, 0, -1);
        setOutputs(topLeftBack, uv00, a_zMinTexture[0], norm);
        setOutputs(bottomLeftBack, uv01, a_zMinTexture[0], norm);
        setOutputs(topLeftFront, uv10, a_zMinTexture[0], norm);
        setOutputs(bottomLeftFront, uv11, a_zMinTexture[0], norm);
        EndPrimitive();
    }

}