#version 460 core

layout (location = 0) in vec3 a_pos;

uniform mat4 u_projT;
uniform mat4 u_viewT;
uniform mat4 u_modelT;

void main()
{
    gl_Position = u_projT * u_viewT * u_modelT * vec4(a_pos, 1.0);
}
