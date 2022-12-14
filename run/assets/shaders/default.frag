#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

vec3 lightDir = vec3(-1, 2, 1);

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main() {
    outColor = texture(texSampler, inUV) * vec4(inColor, 1);

    outColor.rgb *= dot(normalize(inNormal), lightDir) / 4 + .75;
}