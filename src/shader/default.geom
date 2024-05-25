#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 14) out;

uniform mat4 u_projT;
uniform mat4 u_viewT;
uniform mat4 u_modelT;

mat4 PVMT = u_projT * u_viewT * u_modelT;

void drawTriangle(float x, float y, float z) {
    gl_Position = PVMT * (gl_in[0].gl_Position + vec4(x, y, z, 0.0));
    EmitVertex();
}

void main() {

    float cubeDelta = 0.3;

    // This could be optimised but honestly I am surprised I even figured this out on my own.
    // I figured you could do this by mistake because I didn't know what 'triangle_strip' meant haha
    // Additionally, I accidentally figured out how to fill in the bottom.

    // intitial position
    drawTriangle(-cubeDelta, cubeDelta, cubeDelta);
    // Front
    drawTriangle(cubeDelta, cubeDelta, cubeDelta);
    drawTriangle(-cubeDelta, -cubeDelta, cubeDelta);
    drawTriangle(cubeDelta, -cubeDelta, cubeDelta);
    // Right (implicitly 1 draw bottom triangle by switching from + Z -> - Z)
    drawTriangle(cubeDelta, -cubeDelta, -cubeDelta);
    drawTriangle(cubeDelta, cubeDelta, cubeDelta);
    drawTriangle(cubeDelta, cubeDelta, -cubeDelta);
    // Top
    drawTriangle(-cubeDelta, cubeDelta, cubeDelta);
    drawTriangle(-cubeDelta, cubeDelta, -cubeDelta);
    // Left
    drawTriangle(-cubeDelta, -cubeDelta, cubeDelta);
    drawTriangle(-cubeDelta, -cubeDelta, -cubeDelta);
    // Bottom
    drawTriangle(cubeDelta, -cubeDelta, -cubeDelta);
    // Back
    drawTriangle(-cubeDelta, cubeDelta, -cubeDelta);
    drawTriangle(cubeDelta, cubeDelta, -cubeDelta);

    EndPrimitive();
}