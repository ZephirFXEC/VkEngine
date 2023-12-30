#version 460
layout (location = 0) out vec4 outColor;
layout (location = 0) in vec3 fragColor;

void main() {
    // noise is in the range [0, 1]
    float rand = fract(sin(dot(gl_FragCoord.xyz, vec3(12.9898, 78.233, 45.5432))) * 43758.5453);
    normalize(rand);

    // noise is in the range [-0.1, 0.1]
    outColor = vec4(fragColor * rand, 1.0);
}