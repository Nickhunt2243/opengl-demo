#version 460 core
in vec2 g_texCoords;
in vec4 g_colorMap;
in flat vec3 g_norm;
in flat float g_colorScalar;
in flat int g_currTex;
in vec3 g_blockPos;
in vec2 g_chunkPos;
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
                g_chunkPos.x >= 15.95 ||
                g_chunkPos.y >= 15.95
            )
            &&
                g_norm.y == 1.0
            &&
            (
                g_texCoords.x >= .95 ||
                g_texCoords.y >= .95
            )
        )
    {
        FragColor = g_colorScalar * vec4(0.8f, 0.2f, 0.2f, 0.8f);
    }
    else if (
            (
                g_chunkPos.x <= 0.05 ||
                g_chunkPos.y <= 0.05
            )
            &&
                g_norm.y == 1.0
            &&
            (
                g_texCoords.x <= .05 ||
                g_texCoords.y <= .05
            )
        )
    {
        FragColor = g_colorScalar * vec4(0.2f, 0.2f, 0.8f, 0.8f);
    }
    else if (
            u_hasLookAt &&
            ( g_blockPos.x > u_lookAtBlock.x - epsilon && g_blockPos.x < u_lookAtBlock.x + 1 + epsilon ) &&
            ( g_blockPos.y > u_lookAtBlock.y - epsilon && g_blockPos.y < u_lookAtBlock.y + 1 + epsilon ) &&
            ( g_blockPos.z > u_lookAtBlock.z - epsilon && g_blockPos.z < u_lookAtBlock.z + 1 + epsilon ) &&
            (g_texCoords.x > 0.9975 || g_texCoords.y > 0.9975 || g_texCoords.x < 0.0025 || g_texCoords.y < 0.0025 )
        )
    {
        FragColor = g_colorScalar * vec4(0.3f, 0.3f, 0.3f, 1.0f);
    }
    else
    {
        vec4 texColor = texture(textures, vec3(g_texCoords, g_currTex));
        FragColor = g_colorScalar * vec4(g_colorMap.rgb * texColor.rgb, 1.0);
    }
}