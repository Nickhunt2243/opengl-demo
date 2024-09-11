#version 460 core
in vec2 g_texCoords;
in vec4 g_colorMap;
in flat ivec3 g_norm;
in flat float g_colorScalar;
in flat int g_currTex;
in flat ivec3 g_blockPos;

uniform sampler2DArray textures;
uniform sampler2DArray colorMaps;
uniform ivec3 u_lookAtBlock;
uniform bool u_hasLookAt;

out vec4 FragColor;

void main()
{
    if (
            g_norm.y == 1.0 && (
                (g_blockPos.x == 15 && g_texCoords.y >= .95) ||
                (g_blockPos.z == 15 && g_texCoords.x >= .95)
            )
        )
    {
        FragColor = g_colorScalar * vec4(0.8f, 0.2f, 0.2f, 0.8f);
    }
    else if (
            g_norm.y == 1 && (
                (g_blockPos.x == 0.0 && g_texCoords.y <= .05) ||
                (g_blockPos.z == 0.0 && g_texCoords.x <= .05)
            )
        )
    {
        FragColor = g_colorScalar * vec4(0.2f, 0.2f, 0.8f, 0.8f);
    }
    else if (
            u_hasLookAt &&
            ( g_blockPos.x == u_lookAtBlock.x) && ( g_blockPos.y == u_lookAtBlock.y) && ( g_blockPos.z == u_lookAtBlock.z) &&
            (g_texCoords.x > 0.9975 || g_texCoords.y > 0.9975 || g_texCoords.x < 0.0025 || g_texCoords.y < 0.0025 )
        )
    {
        FragColor = g_colorScalar * vec4(0.3f, 0.3f, 0.3f, 1.0f);
    }
    else
    {
        vec4 texColor = texture(textures, vec3(g_texCoords, g_currTex));
        FragColor = g_colorScalar * texColor;
    }
}