#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.
uniform int u_Time;
in vec4 vs_Pos;             // The array of vertex positions passed to the shader
in vec4 vs_Nor;             // The array of vertex normals passed to the shader
in vec4 vs_Col;             // The array of vertex colors passed to the shader.
in vec2 vs_UV;              // The array of vertex texture coordinates passed to the shader
in float vs_Cosine;
in float vs_Animated;

out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.
out vec2 fs_UV;             // The UV of each vertex. This is implicitly passed to the fragment shader.
out float fs_Cosine;
out float fs_Animated;

const vec4 lightDir = vec4(1.2,1,1.4,0);  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

void main()
{
    fs_UV = vs_UV;    // Pass the vertex UVs to the fragment shader for interpolation

    vec2 uv = fs_UV;

    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
    fs_Cosine = vs_Cosine;
    fs_Animated = vs_Animated;

    vec4 pos = vs_Pos;

    vec4 nor = vs_Nor;


    if ((uv.x < 0 || uv.y < 0)) {
        if (fs_Animated >= 2 - 1e-5 && fs_Animated <= 2 + 1e-5) {
            pos.y -= (int(u_Time * 0.45) % 200) * 0.5;
            if (pos.y < 129) {
                pos.y = 100 + pos.y;
            }
        }
        else if (fs_Animated >= 3 - 1e-5 && fs_Animated <= 3 + 1e-5) {
            pos.y -= (int(u_Time * 0.45) % 200) * 0.5;
            if (pos.y < 130) {
                pos.y = 100 + pos.y;
            }
        }
        else if (fs_Animated >= 4 - 1e-5 && fs_Animated <= 4 + 1e-5) {
            pos.y += (int((u_Time / uv.y)) % 2) * 0.8;
        }

        else if (fs_Animated >= 6 - 1e-5 && fs_Animated <= 6 + 1e-5) {
            if (u_Time % 800 > 240 && u_Time % 800 < 256) {
               pos.x -= 2;
               pos.z += 2;
            }
            else if (u_Time % 800 > 260 && u_Time % 800 < 288) {
                pos.x += 2;
                pos.z -= 2;
            }
        } 
    }
    else if (fs_Animated >= 7 - 1e-5 && fs_Animated <= 7 + 1e-5) {
        vec3 direction = cross(vs_Nor.xyz, vec3(0, 1, 0));
        direction = direction * vs_Col.x;
        pos.x += (int(u_Time * 0.45) % int(100.0 / vs_Col.y)) * direction.x;
        pos.z += (int(u_Time * 0.45) % int(100.0 / vs_Col.y)) * direction.z;
        pos.y -= (int(u_Time * 0.45) % int(100.0 / vs_Col.y)) * vs_Col.y;
        if (pos.y < 128) {
            pos.y = 100 + pos.y;
            pos.x -= 100.0 / vs_Col.y * direction.x;
            pos.z -= 100.0 / vs_Col.y * direction.z;
        }
    }
    else if (fs_Animated >= 8 - 1e-5 && fs_Animated <= 8 + 1e-5) {
        vec3 direction = cross(vs_Nor.xyz, vec3(0, 1, 0));
        direction = direction * vs_Col.x;
        pos.x += (int(u_Time * 0.45) % int(100.0 / vs_Col.y)) * direction.x;
        pos.z += (int(u_Time * 0.45) % int(100.0 / vs_Col.y)) * direction.z;
        pos.y -= (int(u_Time * 0.45) % int(100.0 / vs_Col.y)) * vs_Col.y;
        if (pos.y < 128 + 0.3) {
            pos.y = 100 + pos.y;
            pos.x -= 100.0 / vs_Col.y * direction.x;
            pos.z -= 100.0 / vs_Col.y * direction.z;
        }
    }

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.



    vec4 modelposition = u_Model * pos;   // Temporarily store the transformed vertex positions for use below

    fs_LightVec = normalize(lightDir);  // Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
