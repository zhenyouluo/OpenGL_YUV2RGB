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
uniform float zNear;
uniform float zFar;

void main(){
        float m_znear = zNear;
        float m_zfar = zFar;

        // un/normalize disparity
        float temp1 = 1.0/m_znear - 1.0/m_zfar;
        float temp2 = 1.0/m_zfar;
        float depthUnnormalized = 1.0*depth*temp1/(256-1.0)+ temp2;
//        depthUnnormalized = 1/100.0;

        // Projected position in 3D homogeneous coordinates
        vec4 ref_position_world = vec4(K2_inv*vec3(vertexPosition_modelspace[0],vertexPosition_modelspace[1],1),1.0*depthUnnormalized);
        ref_position_world[0] = ref_position_world[0]/ref_position_world[3];
        ref_position_world[1] = ref_position_world[1]/ref_position_world[3];
        ref_position_world[2] = ref_position_world[2]/ref_position_world[3];
//        ref_position_world[2] = -100;
        ref_position_world[3] = 1.0;


        gl_Position =  MVP * ref_position_world;
        /// putting -1.0 there mirrors the image so it is correct. should not be necessary. how to fix?
        gl_Position[0] = gl_Position[0]/gl_Position[3];
        gl_Position[1] = gl_Position[1]/gl_Position[3];
        gl_Position[2] = gl_Position[2]/gl_Position[3];
        gl_Position[3] = 1.0;

        // UV of the vertex. No special space for this one.
        UV = vertexUV;
}
