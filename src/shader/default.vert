#version 460 core

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
