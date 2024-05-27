#version 460 core
in VS_OUT
{
    vec2 v_TexCoord;
    vec4 colorMapping;
    flat int currTex;
} fs_in;

uniform sampler2D textures[6];

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(textures[fs_in.currTex], fs_in.v_TexCoord);
    FragColor = vec4(texColor.xyz * fs_in.colorMapping.rgb, 1.0);
}