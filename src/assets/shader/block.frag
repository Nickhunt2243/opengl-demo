#version 460 core
in flat ivec3 v_blockPos;
in flat ivec2 v_chunkPos;
in flat float v_colorScalar;
in flat ivec3 v_norm;
in vec2 v_uv;
in flat int v_textureInfo;
in float v_ambientValue;

uniform sampler2DArray textures;
uniform sampler2DArray colorMaps;
uniform ivec3 u_lookAtBlock;
uniform bool u_hasLookAt;

out vec4 FragColor;

lowp float ambientScalar;

void main()
{

    //
    // 0 -> .56, 1 -> .69, 2 -> .82, 3 -> 1.0
    ambientScalar = (v_ambientValue * 0.13) + 0.56;
    float lightScalar = ambientScalar * v_colorScalar;

    // Stuff for showing chunk grid lines. Not always needed but leaving for future.
//    if (
//            v_norm.y == 1.0 && (
//                (v_blockPos.x == 15 && v_uv.y >= .9) ||
//                (v_blockPos.z == 15 && v_uv.x >= .9)
//            )
//        )
//    {
//        FragColor = lightScalar * vec4(0.8f, 0.2f, 0.2f, 0.8f);
//    }
//    else if (
//            v_norm.y == 1 && (
//                (v_blockPos.x == 0.0 && v_uv.y <= .1) ||
//                (v_blockPos.z == 0.0 && v_uv.x <= .1)
//            )
//        )
//    {
//        FragColor = lightScalar * vec4(0.2f, 0.2f, 0.8f, 0.8f);
//    }
    // Invalid block (mostly used when a player deletes a block).
    if (v_textureInfo == 0)
    {
        discard;
    }

    if (
            u_hasLookAt &&
            (v_blockPos.x == u_lookAtBlock.x) && (v_blockPos.y == u_lookAtBlock.y) && (v_blockPos.z == u_lookAtBlock.z) &&
            (v_uv.x > 0.9965 || v_uv.y > 0.9965 || v_uv.x < 0.0035 || v_uv.y < 0.0035 )
        )
    {
        float lightConst = lightScalar * 0.2f;
        FragColor = vec4(lightConst, lightConst, lightConst, 1.0f);
    }
    else
    {
        FragColor = vec4(lightScalar * texture(textures, vec3(v_uv, v_textureInfo)).xyz, 1.0);
    }
}