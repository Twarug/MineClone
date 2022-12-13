#version 450

layout(binding = 0) uniform UBO {
    vec4 data;
    mat4 proj;
    mat4 view;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outUV;

void main() {
    gl_Position = ubo.proj * ubo.view * pushConstants.model * vec4(inPosition, 1.0);
    outNormal = inNormal;
    outColor = inColor;
    outUV = inUV; 
}