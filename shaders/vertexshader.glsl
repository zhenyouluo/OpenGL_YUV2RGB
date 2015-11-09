#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in float depth;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat3 K2_inv;
uniform mat3x3 matProjectXY_from_ref_to_virt;
uniform vec3 matProjectZ_from_ref_to_virt;
uniform mat3x4 matProjectXY_from_ref_to_virt_world;
uniform vec4 matProjectZ_from_ref_to_virt_world;

void main(){
        float m_znear = 15;
        float m_zfar = 150;

        // un/normalize disparity
        float temp1 = 1.0/m_znear - 1.0/m_zfar;
        float temp2 = 1.0/m_zfar;
        float depthUnnormalized = 1.0*depth*temp1/(256-1.0)+ temp2;
//        depthUnnormalized = 1/100.0;

        // Projected position. This will be homogeneous 2D coordinates though.
//        vec3 virt_position = vec3(matProjectXY_from_ref_to_virt * vertexPosition_modelspace);// + matProjectZ_from_ref_to_virt*depth;
        // Extend to 3D for OpenGL
        //gl_Position =  vec4(virt_position,1);

        // Projected position in 3D homogeneous coordinates
//        vec4 virt_position_world = vec4(matProjectXY_from_ref_to_virt_world * vec3(vertexPosition_modelspace[0],vertexPosition_modelspace[1],1));// + matProjectZ_from_ref_to_virt*depth;
        vec4 ref_position_world = vec4(K2_inv*vec3(vertexPosition_modelspace[0],vertexPosition_modelspace[1],1),1.0*depthUnnormalized);
        ref_position_world[0] = ref_position_world[0]/ref_position_world[3];
        ref_position_world[1] = ref_position_world[1]/ref_position_world[3];
        ref_position_world[2] = ref_position_world[2]/ref_position_world[3];
//        ref_position_world[2] = -100;
        ref_position_world[3] = 1.0;

//        ref_position_world = vec4(K2_inv*vec3(vertexPosition_modelspace[0],vertexPosition_modelspace[1],1),1.0);
//        ref_position_world[2] = -30;
        // variables have to be used or compiler will remove them, use this for debugging, to check values of variables
//        vec4 test = vec4(matProjectXY_from_ref_to_virt_world * matProjectXY_from_ref_to_virt * matProjectZ_from_ref_to_virt) + matProjectZ_from_ref_to_virt_world ;
        // Output position of the vertex, in clip space : MVP * position
//        gl_Position =  MVP * vec4(vertexPosition_modelspace,1);// + test;
//        gl_Position =  MVP * virt_position_world;
        gl_Position =  MVP * ref_position_world;
        /// putting -1.0 there mirrors the image so it is correct. should not be necessary. how to fix?
        gl_Position[0] = gl_Position[0]/gl_Position[3];
        gl_Position[1] = gl_Position[1]/gl_Position[3];
        gl_Position[2] = gl_Position[2]/gl_Position[3];
        gl_Position[3] = 1.0;

        // UV of the vertex. No special space for this one.
        UV = vertexUV;
}
