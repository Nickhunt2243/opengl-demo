#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 24) out;
out VS_OUT
{
    vec2 v_TexCoord;
    vec4 colorMapping;
    flat int currTex;
} vs_out;

uniform mat4 u_projT;
uniform mat4 u_viewT;
uniform mat4 u_modelT;

int TOP_TEX = 0;
int BOTTOM_TEX = 1;

int FRONT_TEX = 2;
int RIGHT_TEX = 3;
int BACK_TEX = 4;
int LEFT_TEX = 5;


mat4 PVMT = u_projT * u_viewT * u_modelT;

bool hasNeighbor(ivec3 coord)
{
    return texelFetch(cubeMap, coord, 0).r != 0;
}

void drawCoord(float x, float y, float z, float u, float v, int tex, vec4 colorMapping)
{
    gl_Position = PVMT * (gl_in[0].gl_Position + vec4(x, y, z, 0.0));
    vs_out.v_TexCoord = vec2(u, v);
    vs_out.currTex = tex;
    vs_out.colorMapping = colorMapping;
    EmitVertex();
}

void main()
{

    float cubeDelta = 0.5;
    // This could be optimised but honestly I am surprised I even figured this out on my own.
    // I figured you could do this by mistake because I didn't know what 'triangle_strip' meant haha
    // Additionally, I accidentally figured out how to fill in the bottom.

    vec4 defaultColorMapping = vec4(1.0, 1.0, 1.0, 0.0);

    // Front
    if (!hasNeighbor(coord + ivec3(0, 0, 1))) {
        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 0, 0, FRONT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 0, 1, FRONT_TEX, defaultColorMapping);
        drawCoord(cubeDelta, cubeDelta, cubeDelta, 1, 0, FRONT_TEX, defaultColorMapping);
        drawCoord(cubeDelta, -cubeDelta, cubeDelta, 1, 1, FRONT_TEX, defaultColorMapping);
        EndPrimitive();
    }
    // Right
    if (!hasNeighbor(coord + ivec3(1, 0, 0))) {
        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 0, 0, FRONT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 0, 1, FRONT_TEX, defaultColorMapping);
        drawCoord(cubeDelta, cubeDelta, -cubeDelta, 0, 0, RIGHT_TEX, defaultColorMapping);
        drawCoord(cubeDelta, -cubeDelta, -cubeDelta, 0, 1, RIGHT_TEX, defaultColorMapping);
        EndPrimitive();
    }
    // Back
    if (!hasNeighbor(coord + ivec3(0, 0, -1))) {
        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 0, 0, FRONT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 0, 1, FRONT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, cubeDelta, -cubeDelta, 1, 0, BACK_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, -cubeDelta, 1, 1, BACK_TEX, defaultColorMapping);
        EndPrimitive();
    }
    // Left
    if (!hasNeighbor(coord + ivec3(1, 0, 0))) {
        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 0, 0, FRONT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 0, 1, FRONT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 0, 0, LEFT_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 0, 1, LEFT_TEX, defaultColorMapping);
        EndPrimitive();
    }
    // Top
    if (!hasNeighbor(coord + ivec3(0, 1, 0))) {
        vec4 greenColorMapping = vec4(0.5065, 0.8296, 0.2516, 0.8);
        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 1, 0, TOP_TEX, greenColorMapping);
        drawCoord(cubeDelta, cubeDelta, cubeDelta, 1, 1, TOP_TEX, greenColorMapping);
        drawCoord(-cubeDelta, cubeDelta, -cubeDelta, 0, 0, TOP_TEX, greenColorMapping);
        drawCoord(cubeDelta, cubeDelta, -cubeDelta, 0, 1, TOP_TEX, greenColorMapping);
        EndPrimitive();
    }
    // Bottom
    if (!hasNeighbor(coord + ivec3(0, -1, 0))) {
        drawCoord(cubeDelta, -cubeDelta, cubeDelta, 0, 0, BOTTOM_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 1, 0, BOTTOM_TEX, defaultColorMapping);
        drawCoord(cubeDelta, -cubeDelta, -cubeDelta, 0, 1, BOTTOM_TEX, defaultColorMapping);
        drawCoord(-cubeDelta, -cubeDelta, -cubeDelta, 1, 1, BOTTOM_TEX, defaultColorMapping);
        EndPrimitive();
    }
}