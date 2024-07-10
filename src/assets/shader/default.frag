#version 460 core

uniform vec4 u_colorMapping;

out vec4 FragColor;
void main()
{
    FragColor = u_colorMapping;
}