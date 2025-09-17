#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 projection;
} pc;

void main() {
    fragPos = vec3(pc.model * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(pc.model))) * inNormal;
    fragTexCoord = inTexCoord;
    
    gl_Position = pc.projection * pc.view * vec4(fragPos, 1.0);
}
