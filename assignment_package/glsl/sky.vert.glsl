#version 150

in vec4 vs_Pos;
in vec2 vs_UV;              // The array of vertex texture coordinates passed to the shader

out vec2 fs_UV;

void main()
{
    gl_Position = vs_Pos;
    fs_UV = vs_UV;
}
