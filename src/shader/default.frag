#version 460 core
out vec4 FragColor;
uniform vec4 u_ourColor;

void main()
{
    FragColor = u_ourColor;
}