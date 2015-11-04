#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureSampler;

void main(){

        // Output color = color of the texture at the specified UV
        color = texture2D( textureSampler, UV ).rgb;
//        color = vec3(1,0,0);
//        color[2] = color[0];
}
