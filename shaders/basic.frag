#version 460

layout (location=0) in vec3 color;
layout (location=1) in vec2 fragTexCoord;
layout (location=0) out vec4 out_FragColor;

void main() {
    // For now, show a simple pattern that's not orange
    vec2 uv = fragTexCoord;
    
    // Create a simple pattern with different colors
    vec3 finalColor = vec3(0.5, 0.5, 0.5); // Default gray
    
    // Add some variation based on UV coordinates
    if (uv.x > 0.5) {
        finalColor = vec3(0.8, 0.4, 0.2); // Orange only on right half
    } else {
        finalColor = vec3(0.2, 0.4, 0.8); // Blue on left half
    }
    
    // Add some green in the middle
    if (uv.y > 0.3 && uv.y < 0.7) {
        finalColor = mix(finalColor, vec3(0.2, 0.8, 0.2), 0.5);
    }
    
    out_FragColor = vec4(finalColor, 1.0);
}