#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main()
{
    vec3 light = normalize(vec3(1.0, 2.0, 3.0));
    float dist = distance(position, vec3(2.0, 3.0, 5.0));
    vec3 outCol = color * (max(0.0f, dot(normal,light))  * (1.0f / (dist * dist)));

    gl_Position = push.transform * vec4(position, 1.0);
    fragColor = outCol;
}