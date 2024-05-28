#version 460 core
in GS_OUT
{
    vec2 v_TexCoord;
    flat int currTex;
} fs_in;

uniform sampler2D textures[6];
uniform vec4 u_textureColorMapping[6];

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(textures[fs_in.currTex], fs_in.v_TexCoord);
    FragColor = vec4(u_textureColorMapping[fs_in.currTex].rgb * texColor.rgb, 1.0);
}