#version 450

struct CircleData {
    float data[6];
};


layout(push_constant) uniform PushConstants {
    uint w;
    uint h;
} res;

layout(set = 0, binding = 0) readonly buffer DataBuffer {
    CircleData circles[];
};

layout(location = 0) out vec2 outPosition;
layout(location = 1) out float outRadius;
layout(location = 2) out vec3 outColor;

vec2 offsets[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
);

void main() {
    float ratio = float(res.h) / float(res.w);
    CircleData currentCircle = circles[gl_InstanceIndex];
    vec2 pos = vec2(currentCircle.data[0], currentCircle.data[1]);
    float radius = currentCircle.data[2] * 2;
    vec3 color = vec3(currentCircle.data[3], currentCircle.data[4], currentCircle.data[5]);

    vec2 posAdj = vec2((pos.x * 2.0) - 1.0, (pos.y * 2.0) - 1.0);
    gl_Position = vec4(posAdj.x + offsets[gl_VertexIndex].x * radius * ratio, posAdj.y + offsets[gl_VertexIndex].y * radius, 0.0, 1.0);
    outPosition = pos;
    outRadius = radius / 2;
    outColor = color;
}
