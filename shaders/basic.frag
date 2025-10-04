#version 460

layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;

void main() {
    // Use the color from vertex shader (position-based coloring)
    out_FragColor = vec4(color, 1.0);
}