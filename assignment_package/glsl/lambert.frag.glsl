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
uniform int u_BlendType;


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

const float PI = 3.14159265359;
//#define NORMAL_MAP

void main()
{
    // Material base color (before shading)
        //vec4 diffuseColor = fs_Col;

    //calculate the direction of light, changed with time
    vec4 lightDir;
    int time = (u_Time) % 2400;
    float sunAngle = PI*time/1200; //calculate the angle of sun's position in world axis at this time, make it 2400 time a circle
    lightDir = normalize(vec4(-cos(sunAngle), sin(sunAngle)+0.8, 0, 0)); //calculate  center of sun's position at this time
    float diffuseRatio; // equal 0.8 at daytime(sunlight), equal 0.4 at nighttime(moonlight)
    //Day Time
    if (time >= 0 && time < 1200) {
        lightDir = normalize(vec4(-cos(sunAngle), sin(sunAngle)+0.8, 0, 0)); //calculate  center of sun's position at this time
        if (time < 120) {
            diffuseRatio = mix(0.0, 0.8, time/120.0); //moonlight
        } else {
            diffuseRatio = 0.8;
        }
    }


    //Sunset/Dusk
    else if (time >= 1200 && time < 1380) {
        lightDir = normalize(vec4(-cos(sunAngle), sin(sunAngle)+0.8, 0, 0)); //calculate  center of sun's position at this time
        diffuseRatio = mix(0.8, 0.0, (time-1200.0)/180.0); //sunlight
    }


    //Night Time
    else if (time >= 1380 && time < 2220) {
        lightDir = normalize(vec4(cos(sunAngle), -sin(sunAngle)+0.3, 0, 0)); //calculate  center of sun's position at this time
        if (time < 1500) {
            diffuseRatio = mix(0.0, 0.4, (time-1380.0)/120.0); //moonlight
        } else {
            diffuseRatio = 0.4;
        }
    }


    //Sunrise/Dawn
    else if (time >= 2220 && time < 2400){
        lightDir = normalize(vec4(cos(sunAngle), -sin(sunAngle)+0.3, 0, 0)); //calculate  center of sun's position at this time
        diffuseRatio = mix(0.4, 0.0, (time-2220.0)/180.0); //moonlight
    }

    //lightDir = fs_LightVec;

    vec2 uv = fs_UV;
    vec4 diffuseColor;

    if (uv.x < 0 || uv.y < 0) {
        if (fs_Animated >= 5 - 1e-5 && fs_Animated <= 6 + 1e-5) {
                    if((time > 1620 && time < 1636) ||
                        (time > 1640 && time < 1656) ||
                        (time > 1660 && time < 1688) ||
                        (time > 1820 && time < 1836) ||
                        (time > 1840 && time < 1856) ||
                        (time > 1860 && time < 1888)) {
                        out_Col = fs_Col;
                    }
                    else {
                        out_Col = vec4(0, 0, 0, 0);
                    }
        }
        else if (fs_Animated >= 2 - 1e-5 && fs_Animated <= 4 + 1e-5) {
            if (time >= 1300 && time < 23000) {
                out_Col = fs_Col;
            }
            else {
                out_Col = vec4(0, 0, 0, 0);
            }
        }
        else if (fs_Animated >= 9 - 1e-5 && fs_Animated <= 9 + 1e-5) {
            if (time >= 0 && time < 1200) {
                out_Col = vec4(1.0, 1.0, 1.0, 0.8);
            }
            else if (time >= 1200 && time < 1400) {
                float coef = float(time - 1200) / 200.;
                out_Col = mix(vec4(1, 1, 1, 0.8), vec4(1.0, 0.8, 0.8, 0.8), vec4(coef, coef, coef, 1));
            }
            else if (time >= 1400 && time < 1380) {
                out_Col = vec4(1.0, 0.8, 0.8, 0.8);
            }
            else if (time >= 1380 && time < 1580) {
                float coef = float(time - 1380) / 200.;
                out_Col = mix(vec4(1.0, 0.8, 0.8, 0.8), vec4(0.4, 0.4, 0.5, 0.8), vec4(coef, coef, coef, 1));
            }
            else if (time >= 1580 && time < 2220) {
                out_Col = vec4(0.4, 0.4, 0.5, 0.8);
            }
            else if (time >= 2220 && time < 2400) {
                float coef = float(time - 2220) / 180.;
                out_Col = mix(vec4(0.4, 0.4, 0.5, 0.8), vec4(1.0, 1.0, 1.0, 0.8), vec4(coef, coef, coef, 1));
            }
            else {
                out_Col = vec4(1.0, 1.0, 1.0, 0.8);
            }
        }
        else if (fs_Animated >= 2 - 1e-5) {
            out_Col = fs_Col;
        }
        else {
            out_Col = vec4(1, 1, 1, 0.8);
        }
    }
    else {
        if (fs_Animated >= 1 - 1e-5 && fs_Animated <=1 + 1e-5) {
            float offset = (int(u_Time * 0.1) % 16) / 256.0;
            uv.x += offset;
        }
        diffuseColor = texture(u_Texture, uv);

        vec4 normal = texture(u_NormalMap, uv);

    #ifdef NORMAL_MAP
        normal = (normal - vec4(0.5)) * 2.0;

        if (fs_Nor.z < -1e-7) {
            normal.z = -normal.z;
        }
        else if (fs_Nor.y > 1e-7) {
            float t = normal.y;
            normal.y = normal.z;
            normal.z = -t;
        }
        else if (fs_Nor.y < -1e-7) {
            float t = normal.y;
            normal.y = -normal.z;
            normal.z = t;
        }
        else if (fs_Nor.x > 1e-7) {
            float t = normal.x;
            normal.x = normal.z;
            normal.z = -t;
        }
        else if (fs_Nor.x < -1e-7) {
            float t = normal.x;
            normal.x = -normal.z;
            normal.z = t;
        }
normal.w = 0.0;
    float diffuseTerm = dot(normalize(normal), normalize(lightDir));
#endif

#ifndef NORMAL_MAP
    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(lightDir));
    // Avoid negative lighting values
#endif
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.4;

    float lightIntensity = diffuseTerm*diffuseRatio  + ambientTerm;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.

    #ifdef NORMAL_MAP
        float specular = max(pow(dot(normalize(normal), normalize(fs_LightVec)), fs_Cosine), 0);
    #endif

    #ifndef NORMAL_MAP
        float specular = max(pow(dot(normalize(fs_Nor), normalize(fs_LightVec)), fs_Cosine), 0);
    #endif
        // Compute final shaded color
        out_Col = vec4(diffuseColor.rgb * lightIntensity + specular, diffuseColor.a);

    }



    if (u_Envir == 1) {//water
        float alpha = 0.1;
        out_Col = out_Col * (1.0 - alpha) + vec4(0, 0, 255, 1) * alpha;
    } else if (u_Envir == 2) {//lava
        float alpha = 0.3;
        out_Col = out_Col * (1.0 - alpha) + vec4(255, 0, 0, 1) * alpha;
    } else {
        if (fs_Animated >= 7 - 1e-5 && fs_Animated <= 8 + 1e-5) {
            out_Col = texture(u_Texture, uv);
        } else if (u_BlendType == 1 && (time > 1300 && time < 2300) && int(u_Time * 0.1) % 17 >= 0 && int(u_Time * 0.1) % 17 <= 3) {
            float alpha = 0.6;
            out_Col = out_Col * (1.0 - alpha) + vec4(0.28, 0.44, 0.76, 1) * alpha;
        } else if (u_BlendType == 2 && int(u_Time * 0.1) % 27 >= 0 && int(u_Time * 0.1) % 27 <= 3) {
            float alpha = 0.8;
            out_Col = out_Col * (1.0 - alpha) + vec4(1., 1., 1., 1.) * alpha;
        }
    }
}


