// Geometry shader example
#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Inputs from the vertex shader
in vec2 texCoords[];        
in int currTex[];
in flat vec4 colorMap[];
in vec3 a_blockPos[];
in vec2 a_chunkPos[];
in flat vec3 a_norm[];
in flat float a_colorScalar[];

// Outputs to the fragment shader
out vec2 g_texCoords;
out flat int g_currTex;
out flat vec4 g_colorMap;
out vec3 g_blockPos;
out vec2 g_chunkPos;
out flat vec3 g_norm;
out flat float g_colorScalar;


// Uniform from your application
uniform vec3 u_CameraNorm;

void main()
{
    // Compare the normal from the first vertex with the camera normal
    if (a_norm[0] != u_CameraNorm) {
        for (int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position;
            g_texCoords = texCoords[i];
            g_currTex = currTex[i];
            g_colorMap = colorMap[i];
            g_blockPos = a_blockPos[i];
            g_chunkPos = a_chunkPos[i];
            g_norm = a_norm[i];
            g_colorScalar = a_colorScalar[i];
            // Optionally pass through other outputs to the fragment shader
            EmitVertex();
        }
        EndPrimitive();
    }
}