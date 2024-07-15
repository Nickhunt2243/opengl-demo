#version 460 core
in vec2 screenCoords;


//uniform vec4 u_colorMapping;
uniform sampler2D u_texture;
uniform vec2 u_screenDimensions;

out vec4 FragColor;
void main()
{
    vec2 normalizedUV = vec2(screenCoords.x / u_screenDimensions.x, screenCoords.y / u_screenDimensions.y) + 0.5;
    vec4 texColor = texture(u_texture, normalizedUV);
    FragColor = vec4(1.0 - texColor.rgb, 1.0);
}
