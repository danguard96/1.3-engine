#version 460

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 texCoord;

layout (location=0) out vec3 fragPos;
layout (location=1) out vec3 fragNormal;
layout (location=2) out vec2 fragTexCoord;

// Push constants for MVP matrices and lighting
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 viewPos;
} pc;

void main() {
    // Transform position to world space
    vec4 worldPos = pc.model * vec4(position, 1.0);
    fragPos = worldPos.xyz;
    
    // Transform normal to world space (inverse transpose of model matrix)
    mat3 normalMatrix = mat3(transpose(inverse(pc.model)));
    fragNormal = normalize(normalMatrix * normal);
    
    // Pass through texture coordinates
    fragTexCoord = texCoord;
    
    // Apply MVP transformation
    gl_Position = pc.proj * pc.view * worldPos;
}

