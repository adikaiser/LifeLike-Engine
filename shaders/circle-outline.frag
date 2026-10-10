#version 450

layout(push_constant) uniform PushConstants {
    uint w;
    uint h;
} res;

layout(location = 0) in vec2 pos;
layout(location = 1) in float radius;
layout(location = 2) in vec3 color;

layout(location = 0) out vec4 outColor;

void main() {
    float ratio = float(res.w) / float(res.h);
    vec2 fragPos = vec2(gl_FragCoord.x / res.w, gl_FragCoord.y / res.h);
    float dx = (pos.x - fragPos.x) * ratio;
    float dy = pos.y - fragPos.y;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist <= radius && dist >= radius - 2.0 / res.h) {
        outColor = vec4(color, 1.0);
    } else {
        outColor = vec4(0.0);
    }
}
