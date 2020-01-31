#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions
uniform int u_BlendType;

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

uniform int u_Envir;

in vec2 fs_UV;

out vec4 outColor;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

//sky color
const vec4 daySky = vec4(0.4f, 0.8f, 1.0f, 1.0);
const vec4 nightSky = vec4(0.0, 0.0, 0.05f, 1.0);
//dusk
const vec4 duskCol = vec4(0.55f, 0.26, 0.22f, 1.0);
const vec4 duskSunCol = vec4(0.9f, 0.8, 0.6f, 1.0);
const vec4 dusksky = vec4(0.24, 0.17, 0.22, 1.0);

const float level1 = 600.0;
const float level2 = 300.0;
//dawn
const vec4 dawnCol = vec4(0.45f, 0.26, 0.22f, 1.0);
const vec4 dawnSunCol = vec4(0.97f, 0.8, 0.36f, 1.0);
const vec4 dawnsky = vec4(0.45f, 0.36f, 0.28f, 1.0);
//
const vec4 sunCol = vec4(1.f, 1.f, 0.9f, 1.0);
const vec4 moonCol = vec4(1.f, 1.f, 0.95, 1.0);


vec3 random3(vec3 p) {
    return fract(sin(vec3(dot(p, vec3(327.1, 211.7, 256.7)),
                          dot(p, vec3(137.1, 341.7, 259.7)),
                          dot(p, vec3(229.5,83.3, 143.0)))) * 13758.5453);
}


float WorleyNoise(vec3 xyz) {
    xyz *= 30.0; // Now the space is 10x10 instead of 1x1. Change this to any number you want.
    vec3 xyzInt = floor(xyz);
    vec3 xyzFract = fract(xyz);
    float minDist = 1.0; // Minimum distance initialized to max.
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            for (int z = -1; z <= 1; ++z) {
                vec3 neighbor = vec3(float(x), float(y), float(z)); // Direction in which neighbor cell lies
                vec3 point = random3(xyzInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
                vec3 diff = neighbor + point - xyzFract; // Distance between fragment coord and neighbor’s Voronoi point
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

#define RAY_AS_COLOR

void main()
{

    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);
    int timeInt = (u_Time) % 2400;
    float time = float(timeInt);
    vec4 skyColor;

    //sun
    float sunAngle = PI*time/1200; //calculate the angle of sun's position in world axis at this time, make it 2400 time a circle
    vec3 curDir = normalize(p.xyz); //calculate the angle fragment's position at this time
    vec3 sunDir = normalize(vec3(-cos(sunAngle), sin(sunAngle)+0.8, 0.0)); //calculate  center of sun's position at this time
    float sunSize = 15; //the size of sun including halo
    float angle = acos(dot(curDir, sunDir)) * 360.0 / PI; //calculate the angle between sun and fragment


    //moon
    float moonAngle = PI*time/1200.0; //calculate the angle of moon's position in world axis at this time, make it 2400 time a circle
    vec3 moonDir = normalize(vec3(cos(moonAngle), -sin(moonAngle)+0.3, 0.0)); //calculate  center of moon's position at this time
    float moonSize = 3.0; //the size of moon including halo
    float angle2 = acos(dot(curDir, moonDir)) * 360.0 / PI; //calculate the angle between moon and fragment

    //Day Time
    if (time >= 0.0 && time < 1200.0) {
        if (time < 150.0) {
            float blend = time/150.0; //blend ratio
            skyColor = dawnsky * (1-blend) + daySky * blend; //sky's current color

            if (p.x < 0.0) {
                if (p.y < level1) { //dawn
                    skyColor = mix(dawnCol, skyColor, blend);
                    if (p.y > level2) { //blend region
                        vec4 tmp = dawnsky * (1-blend) + daySky * blend;
                        float heightRatio = (p.y - level2)/(level1 - level2);
                        skyColor = mix(skyColor, tmp, heightRatio);
                    }
                }
                if (p.x > -500.0) {
                    float mixR = -p.x/500.0;
                    vec4 tmp = dawnsky * (1-blend) + daySky * blend;
                    skyColor = mix(tmp, skyColor, mixR);
                }
            }
        } else {
            skyColor = daySky;
        }


        //During daytime, sun moves from east to west(which is in xy plane)
        if (angle < sunSize) {
            if(angle < 2.0) { //sun's size excluding halo
                outColor = sunCol;
            } else { //calculate halo
                outColor = mix(sunCol, skyColor, (angle - 2.0) / 13.0);
            }
        } else {
            outColor = skyColor;
        }

    }


    //Sunset/Dusk
    else if (time >= 1200.0 && time < 1380.0) {
        float blend = (time - 1200.0)/(1380.0-1200.0); //blend ratio
        skyColor = daySky * (1.0-blend) + dusksky * blend; //sky's current color
        //if sky is in the dusk position
        if (p.x > 0.0) {
            if (p.y < level1) {
                skyColor = mix(skyColor, duskCol, blend);
                if (p.y > level2) {
                    vec4 tmp = daySky * (1.0-blend) + dusksky * blend;
                    float heightRatio = (p.y - level2)/(level1 - level2);
                    skyColor = mix(skyColor, tmp, heightRatio);
                }
            }
            if (p.x < 500.0) {
                float mixR = p.x/500.0;
                vec4 tmp = daySky * (1.0-blend) + dusksky * blend;
                skyColor = mix(tmp, skyColor, mixR);
            }
        }
        //calculate sun's color
        if (angle < 15.0 - blend * 5.0) {
            vec3 cursunCol = vec3(duskSunCol)*blend + vec3(sunCol) * (1-blend);
            if(angle < 2.0 + blend * 5.0) { //sun's size excluding halo
                outColor = vec4(cursunCol, 1.0);
            } else { //calculate halo
                outColor = mix(vec4(cursunCol, 1.0), skyColor, (angle - 2.0 - blend * 5.0) / (13.0 - 10.0 * blend));
            }
        } else {
            outColor = skyColor;
        }
    }


    //Night Time
    else if (time >= 1380.0 && time < 2220.0) {
        float blend = (time - 1380.0)/(1600.0-1380.0); //blend ratio
        skyColor = dusksky * (1.0-blend) + nightSky * blend; //sky's current color

        if (p.x > 0.0) {
            if (p.y < level1 - blend * 200.0) {
                skyColor = mix(duskCol, skyColor, blend);
                if (p.y > level2 - blend * 200.0) {
                    vec4 tmp =  dusksky * (1.0-blend) + nightSky * blend;
                    float heightRatio = (p.y - level2 + blend * 200.0)/(level1 - level2);
                    skyColor = mix(skyColor, tmp, heightRatio);
                }
            }
            if (p.x < 500.0) {
                float mixR = p.x/500.0;
                vec4 tmp =  dusksky * (1.0 - blend) + nightSky * blend;
                skyColor = mix(tmp, skyColor, mixR);
            }
        }

        if (angle < 10.0) {
            if(angle < 7.0) { //sun's size excluding halo
                outColor = duskSunCol * (1.0 - blend) + skyColor * blend;;
            } else { //calculate halo
                outColor = mix(duskSunCol, skyColor, (angle - 7.0) / 3.0);
                outColor = outColor * (1.0 - blend) + skyColor * blend;
            }
        } else {
            outColor = skyColor;
        }


        //moon
        if(angle2 < moonSize) { //sun's size excluding halo
            outColor = moonCol;
        }

        //star
        float dis = WorleyNoise(normalize(p.xyz));
        if (dis < 0.03) {
            outColor = moonCol;
        }

    }


    //Sunrise/Dawn
    else if (time >= 2220.0 && time < 2400.0){
        float blend = (time - 2220.0)/(2400.0-2220.0);
        skyColor = nightSky * (1.0 -blend) + dawnsky * blend;

        //if sky is in the dawn position
        if (p.x < 0.0) {
            if (p.y < level1) {
                skyColor = mix(skyColor, dawnCol, blend);
                if (p.y > level2) {
                    vec4 tmp = nightSky * (1.0 -blend) + dawnsky * blend;
                    float heightRatio = (p.y - level2)/(level1 - level2);
                    skyColor = mix(skyColor, tmp, heightRatio);
                }
            }
            if (p.x > -500.0) {
                float mixR = -p.x/500.0;
                vec4 tmp = nightSky * (1.0 -blend) + dawnsky * blend;
                skyColor = mix(tmp, skyColor, mixR);
            }
        }

        //calculate sun's color
        if (angle < 15.0 - (1.0 -blend) * 5.0) {
            vec3 cursunCol = vec3(duskSunCol)*(1.0-blend) + vec3(sunCol) * blend;
            if(angle < 2.0 + (1.0 -blend) * 5.0) { //sun's size excluding halo
                outColor = vec4(cursunCol, 1.0);
            } else { //calculate halo
                outColor = mix(vec4(cursunCol, 1.0), skyColor, (angle - 2.0 - (1-blend) * 5.0) / (13.0 - 10.0 * (1-blend)));
            }
        } else {
            outColor = skyColor;
        }


        //moon
        if(angle2 < moonSize) { //sun's size excluding halo
            outColor = mix(moonCol, outColor, blend);
        }
    }

    if (u_Envir == 1) {//water
        float alpha = 0.001;
        outColor = outColor * (1.0 - alpha) + vec4(0, 0, 255, 1) * alpha;
    } else if (u_Envir == 2) {//lava
        float alpha = 0.3;
        outColor = outColor * (1.0 - alpha) + vec4(255, 0, 0, 1) * alpha;
    }

    else if( u_BlendType == 3 &&
        ((timeInt > 1620. && timeInt < 1636.) ||
        (timeInt > 1640. && timeInt < 1656.) ||
        (timeInt > 1660. && timeInt < 1688.) ||
        (timeInt > 1820. && timeInt < 1836.) ||
        (timeInt > 1840. && timeInt < 1856.) ||
        (timeInt > 1860. && timeInt < 1888.))) {
        float alpha = 0.3;
        outColor = outColor * (1.0 - alpha) + vec4(1.0, 1.0, 1.0, 1.0) * alpha;
    }
    else if (u_BlendType == 1 && (time > 1300.0 && time < 2300.0) && int(u_Time * 0.1) % 17 >= 0. && int(u_Time * 0.1) % 17 <= 3.) {
        float alpha = 0.6;
        outColor = outColor * (1.0 - alpha) + vec4(0.28, 0.44, 0.76, 1.0) * alpha;
    }
    else if (u_BlendType == 2 && int(u_Time * 0.1) % 27 >= 0. && int(u_Time * 0.1) % 27 <= 3.) {
        float alpha = 0.8;
        outColor = outColor * (1.0 - alpha) + vec4(1., 1., 1., 1.) * alpha;
    }

}
