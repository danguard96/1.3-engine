#version 460

layout (location=0) in vec3 position;

layout (location=0) out vec3 color;

// Push constants for MVP matrices
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 proj;
} pc;

void main() {
    // Scale down the huge glTF coordinates (eight times the original size)
    vec3 scaledPos = position * 0.0008; // Scale down by 1250x (eight times as big)
    
    // Apply MVP transformation
    vec4 worldPos = pc.model * vec4(scaledPos, 1.0);
    vec4 viewPos = pc.view * worldPos;
    gl_Position = pc.proj * viewPos;
    
    // No texture coordinates in simplified version
    
    // Use position-based coloring to see the geometry
    color = vec3(
        abs(scaledPos.x) * 5.0,
        abs(scaledPos.y) * 5.0, 
        abs(scaledPos.z) * 5.0
    );
}   