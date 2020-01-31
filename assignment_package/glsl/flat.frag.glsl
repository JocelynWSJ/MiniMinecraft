#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

uniform int u_Envir;
in vec4 fs_Col;

out vec4 out_Col;

void main()
{
    // Copy the color; there is no shading.
    out_Col = fs_Col;

    if (u_Envir == 1) {//water
            float alpha = 0.001;
            out_Col = out_Col * (1.0 - alpha) + vec4(0, 0, 255, 1) * alpha;
        } else if (u_Envir == 2) {//lava
            float alpha = 0.3;
            out_Col = out_Col * (1.0 - alpha) + vec4(255, 0, 0, 1) * alpha;
        }
}
