#version 460 core

layout (location = 0) in vec2 a_pos;

uniform mat4 u_projT;
uniform mat4 u_modelT;

out vec2 screenCoords;

void main()
{
    screenCoords = a_pos;
    gl_Position = u_projT * u_modelT * vec4(a_pos, 0.0f, 1.0);
}
