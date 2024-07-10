#version 460 core
in vec2 texCoords;
in vec4 colorMap;
in flat vec3 a_norm;
in flat float a_colorScalar;
in flat int currTex;
uniform sampler2DArray textures;
uniform sampler2DArray colorMaps;

in vec2 a_blockPos;

out vec4 FragColor;

void main()
{
    if (
        (
            a_blockPos.x >= 15.95 ||
            a_blockPos.y >= 15.95
        ) &&
        a_norm.y == 1.0 &&
        (
            texCoords.x >= .95 ||
            texCoords.y >= .95
        )
    )
    {
        FragColor = a_colorScalar * vec4(0.8f, 0.2f, 0.2f, 0.8f);
    }
    else if (texCoords.x > .995 || texCoords.y >= .995 || texCoords.x < .005 || texCoords.y < .005)
    {
        FragColor = a_colorScalar * vec4(0.2f, 0.2f, 0.2f, 0.8f);
    }
    else
    {
        vec4 texColor = texture(textures, vec3(texCoords, currTex));
        FragColor = a_colorScalar * vec4(colorMap.rgb * texColor.rgb, 1.0);
    }
}