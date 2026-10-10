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
    float dx = (fragPos.x - pos.x) * ratio;
    float dy = fragPos.y - pos.y;
    float borderSize = 2.0 / res.h;
    if (dx < borderSize || dx > size.x - borderSize || dy < borderSize || dy > size.y - borderSize) {
        outColor = vec4(color, 1.0);
    } else {
        outColor = vec4(0.0);
    }
}


