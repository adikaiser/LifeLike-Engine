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
    float ratio = float(res.w) / float(res.h);
    vec2 fragPos = vec2(gl_FragCoord.x / res.w, gl_FragCoord.y / res.h);
    vec2 halveSize = size / 2.0;
    vec2 centerPos = pos + halveSize;
    float dx = abs(fragPos.x - centerPos.x) * ratio;
    float dy = abs(fragPos.y - centerPos.y);
    float valX = pow(clamp(halveSize.x - dx, 0.0, 1.0) / halveSize.x, 2);
    float valY = pow(clamp(halveSize.y - dy, 0.0, 1.0) / halveSize.y, 2);
    outColor = vec4(color, min(valX, valY));
}


