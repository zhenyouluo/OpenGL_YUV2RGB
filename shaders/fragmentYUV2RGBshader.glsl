#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureSamplerRed;
uniform sampler2D textureSamplerGreen;
uniform sampler2D textureSamplerBlue;

void main(){

    // ITU-R BT 709 to RGB mapping matrix
    mat3 mat709toRGB = mat3(1.164, 1.164, 1.164,  // definition looks transposed
                            0.0, -0.213, 2.112,   // since openGL uses column-major order
                            1.793, -0.533, 0.0);

    // Output color = color of the texture at the specified UV
    vec3 color709;
    color709.r = texture2D( textureSamplerRed, UV ).r;
    color709.g = texture2D( textureSamplerGreen, UV ).r;
    color709.b = texture2D( textureSamplerBlue, UV ).r;

    // make sure input has correct range
    color709.r = clamp(color709.r, 16.0/255, 235.0/255); // Y
    color709.g = clamp(color709.g, 16.0/255, 240.0/255); // U
    color709.b = clamp(color709.b, 16.0/255, 240.0/255); // V

    // de/normalization
    color709.r = color709.r - 0.0625;
    color709.g = color709.g - 0.5;
    color709.b = color709.b - 0.5;

    // finally the conversion
    vec3 rgb = clamp(mat709toRGB * color709, 0.0, 1.0);

    color = rgb;
//    color = texture2D( textureSampler, UV ).rgb;
}
