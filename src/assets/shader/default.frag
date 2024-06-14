#version 460 core
in vec2 texCoords;
in vec4 colorMap;
in flat int currTex;

uniform sampler2DArray textures;
uniform sampler2DArray colorMaps;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(textures, vec3(texCoords, currTex));
    FragColor = vec4(colorMap.rgb * texColor.rgb, 1.0);
}