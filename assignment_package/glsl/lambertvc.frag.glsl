#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform int u_Time;
uniform int u_Envir;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec2 fs_UV;
in float fs_Cosine;
in float fs_Animated;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

uniform sampler2D u_Texture;
uniform sampler2D u_NormalMap;

//#define NORMAL_MAP

void main()
{
    // use vertex color as diffuse color
    vec4 diffuseColor = fs_Col;

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.5;

    float lightIntensity = diffuseTerm * 0.5 + ambientTerm; //Add a small float value to the color multiplier
                                                            //to simulate ambient lighting. This ensures that faces that are not
                                                            //lit by our point light are not completely black.

    float specular = max(pow(dot(normalize(fs_Nor), normalize(fs_LightVec)), fs_Cosine), 0);
    // Compute final shaded color
    out_Col = vec4(diffuseColor.rgb * lightIntensity + specular, diffuseColor.a);
    // if (u_Envir == 1) {//water
    //     float alpha = 0.001;
    //     out_Col = out_Col * (1.0 - alpha) + vec4(0, 0, 255, 1) * alpha;
    // } else if (u_Envir == 2) {//lava
    //     float alpha = 0.3;
    //     out_Col = out_Col * (1.0 - alpha) + vec4(255, 0, 0, 1) * alpha;
    // }
}
