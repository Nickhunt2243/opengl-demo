#version 460 core
in vec2 texCoords;
in vec4 colorMap;
in flat vec3 a_norm;
in flat float a_colorScalar;
in flat int currTex;
in vec4 newPos;
in vec3 a_blockPos;
in vec2 a_chunkPos;
uniform sampler2DArray textures;
uniform sampler2DArray colorMaps;
uniform vec3 u_lookAtBlock;
uniform bool u_hasLookAt;

out vec4 FragColor;

void main()
{
    const float epsilon = 0.001;
    if (
            (
                a_chunkPos.x >= 15.95 ||
                a_chunkPos.y >= 15.95
            )
            &&
                a_norm.y == 1.0
            &&
            (
                texCoords.x >= .95 ||
                texCoords.y >= .95
            )
        )
    {
        FragColor = a_colorScalar * vec4(0.8f, 0.2f, 0.2f, 0.8f);
    }
    else if (
            (
                a_chunkPos.x <= 0.05 ||
                a_chunkPos.y <= 0.05
            )
            &&
                a_norm.y == 1.0
            &&
            (
                texCoords.x <= .05 ||
                texCoords.y <= .05
            )
        )
    {
        FragColor = a_colorScalar * vec4(0.2f, 0.2f, 0.8f, 0.8f);
    }
    else if (
            u_hasLookAt &&
            ( a_blockPos.x > u_lookAtBlock.x - epsilon && a_blockPos.x < u_lookAtBlock.x + 1 + epsilon ) &&
            ( a_blockPos.y > u_lookAtBlock.y - epsilon && a_blockPos.y < u_lookAtBlock.y + 1 + epsilon ) &&
            ( a_blockPos.z > u_lookAtBlock.z - epsilon && a_blockPos.z < u_lookAtBlock.z + 1 + epsilon ) &&
            (texCoords.x > 0.9975 || texCoords.y > 0.9975 || texCoords.x < 0.0025 || texCoords.y < 0.0025 )
        )
    {
        FragColor = a_colorScalar * vec4(0.3f, 0.3f, 0.3f, 1.0f);
    }
    else
    {
        vec4 texColor = texture(textures, vec3(texCoords, currTex));
        FragColor = a_colorScalar * vec4(colorMap.rgb * texColor.rgb, 1.0);
    }
}