#version 450

struct RectData {
    float data[7];
};


layout(push_constant) uniform PushConstants {
    uint w;
    uint h;
} res;

layout(set = 0, binding = 0) readonly buffer DataBuffer {
    RectData rects[];
};

layout(location = 0) out vec2 outPosition;
layout(location = 1) out vec2 outSize;
layout(location = 2) out vec3 outColor;

vec2 offsets[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

void main() {
    float ratio = float(res.h) / float(res.w);
    RectData currentRect = rects[gl_InstanceIndex];
    vec2 pos = vec2(currentRect.data[0], currentRect.data[1]);
    vec2 size = vec2(currentRect.data[2] * 2, currentRect.data[3] * 2);
    vec3 color = vec3(currentRect.data[4], currentRect.data[5], currentRect.data[6]);

    vec2 posAdj = vec2((pos.x * 2.0) - 1.0, (pos.y * 2.0) - 1.0);
    gl_Position = vec4(posAdj.x + offsets[gl_VertexIndex].x * size.x * ratio, posAdj.y + offsets[gl_VertexIndex].y * size.y, 0.0, 1.0);
    //gl_Position = vec4(posAdj + offsets[gl_VertexIndex], 0.0, 1.0);
    outPosition = pos;
    outSize = size / 2;
    outColor = color;
}
