//#version 460 core
//
//layout (points) in;
//layout (triangle_strip, max_vertices = 24) out;
//
//in VS_OUT {
//    vec4 textureColorMapping[6];
//} gs_in[];
//
//out GS_OUT {
//    vec2 v_TexCoord;
//    flat int currTex;
//} gs_out;
//
//uniform mat4 u_projT;
//uniform mat4 u_viewT;
//uniform mat4 u_modelT;
//
//uniform bool drawFace[6];
//
//const int TOP_TEX = 0;
//const int BOTTOM_TEX = 1;
//const int FRONT_TEX = 2;
//const int RIGHT_TEX = 3;
//const int BACK_TEX = 4;
//const int LEFT_TEX = 5;
//
//mat4 PVMT = u_projT * u_viewT * u_modelT;
//
//void drawCoord(float x, float y, float z, float u, float v, int tex)
//{
//    gl_Position = PVMT * (gl_in[0].gl_Position + vec4(x, y, z, 0.0));
//    gs_out.v_TexCoord = vec2(u, v);
//    gs_out.currTex = tex;
//    EmitVertex();
//}
//
//void main()
//{
//    vec3 coord = gl_in[0].gl_Position.xyz;
//    float cubeDelta = 0.5;
//
//    // Top
//    if (drawFace[0])
//    {
//        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 1, 0, TOP_TEX);
//        drawCoord(cubeDelta, cubeDelta, cubeDelta, 1, 1, TOP_TEX);
//        drawCoord(-cubeDelta, cubeDelta, -cubeDelta, 0, 0, TOP_TEX);
//        drawCoord(cubeDelta, cubeDelta, -cubeDelta, 0, 1, TOP_TEX);
//        EndPrimitive();
//    }
//    // Bottom
//    if (drawFace[1])
//    {
//        drawCoord(cubeDelta, -cubeDelta, cubeDelta, 0, 0, BOTTOM_TEX);
//        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 1, 0, BOTTOM_TEX);
//        drawCoord(cubeDelta, -cubeDelta, -cubeDelta, 0, 1, BOTTOM_TEX);
//        drawCoord(-cubeDelta, -cubeDelta, -cubeDelta, 1, 1, BOTTOM_TEX);
//        EndPrimitive();
//    }
//    // Front
//    if (drawFace[2])
//    {
//        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 0, 0, FRONT_TEX);
//        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 0, 1, FRONT_TEX);
//        drawCoord(cubeDelta, cubeDelta, cubeDelta, 1, 0, FRONT_TEX);
//        drawCoord(cubeDelta, -cubeDelta, cubeDelta, 1, 1, FRONT_TEX);
//        EndPrimitive();
//    }
//    // Right
//    if (drawFace[3])
//    {
//        drawCoord(cubeDelta, cubeDelta, cubeDelta, 0, 0, RIGHT_TEX);
//        drawCoord(cubeDelta, -cubeDelta, cubeDelta, 0, 1, RIGHT_TEX);
//        drawCoord(cubeDelta, cubeDelta, -cubeDelta, 1, 0, RIGHT_TEX);
//        drawCoord(cubeDelta, -cubeDelta, -cubeDelta, 1, 1, RIGHT_TEX);
//        EndPrimitive();
//    }
//    if (drawFace[4])
//    {
//        // Back
//        drawCoord(cubeDelta, cubeDelta, -cubeDelta, 0, 0, BACK_TEX);
//        drawCoord(cubeDelta, -cubeDelta, -cubeDelta, 0, 1, BACK_TEX);
//        drawCoord(-cubeDelta, cubeDelta, -cubeDelta, 1, 0, BACK_TEX);
//        drawCoord(-cubeDelta, -cubeDelta, -cubeDelta, 1, 1, BACK_TEX);
//        EndPrimitive();
//    }
//    // Left
//    if (drawFace[5])
//    {
//        drawCoord(-cubeDelta, cubeDelta, -cubeDelta, 0, 0, LEFT_TEX);
//        drawCoord(-cubeDelta, -cubeDelta, -cubeDelta, 0, 1, LEFT_TEX);
//        drawCoord(-cubeDelta, cubeDelta, cubeDelta, 1, 0, LEFT_TEX);
//        drawCoord(-cubeDelta, -cubeDelta, cubeDelta, 1, 1, LEFT_TEX);
//        EndPrimitive();
//    }
//
//}
