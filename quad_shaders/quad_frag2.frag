#version 450
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0, set = 0) readonly buffer data0 { vec4 a_dataFullRes[]; }; // 
layout(push_constant) uniform PushConstants {
    ivec2 resolution;  // width, height 
} pushConsts;

void main() {
    vec2 uv = inUV;  // [0,1]
    ivec2 coord = ivec2(uv * vec2(pushConsts.resolution));
    int index = coord.y * pushConsts.resolution.x + coord.x; 
    outColor = a_dataFullRes[index]; // show blue for debug: mul with vec4(0.1,0.1,1,1) for example;
}
