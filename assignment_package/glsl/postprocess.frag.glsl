in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    color = texture(u_RenderedTexture, fs_UV).rgb;
}
