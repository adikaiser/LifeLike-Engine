#version 450

layout(push_constant) uniform PushConstants {
    uint w;
    uint h;
} res;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 size;
layout(location = 2) in vec3 color;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(color, 1.0);
}

