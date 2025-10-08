#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_FragColor;

// Real time uniform
layout(push_constant) uniform PushConstants {
    uint texColor;
    uint smpl;
    float time;  // Real time in seconds
} pc;

// Manually define the bindless texture arrays
layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

// Manually define the textureBindless2D function
vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
  return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

// Warping function
vec2 dreamWarp(vec2 uv) {
    vec2 center = vec2(0.5);
    vec2 toCenter = center - uv;
    float dist = length(toCenter);

        // Create a pulsing effect
        float pulseSpeed = 2.0;
        float pulseAmount = 0.02;
        float pulse = sin(pc.time * pulseSpeed) * pulseAmount;

    // Create a spiral warp
    float angle = atan(toCenter.y, toCenter.x);
    float spiralAmount = 0.1;
    float spiral = dist * spiralAmount;

    // Combine effects
    vec2 warped = uv + normalize(toCenter) * (pulse + spiral);

        // Add wave distortion
        warped.x += sin(warped.y * 10.0 + pc.time * 3.0) * 0.02;
        warped.y += cos(warped.x * 8.0 + pc.time * 2.0) * 0.02;

    return warped;
}

// Color manipulation
vec3 dreamColor(vec3 color) {
    // Enhance certain colors
    vec3 highlight = vec3(0.7, 0.9, 1.0);  // Slight blue tint
    color = mix(color, highlight, 0.2);

        // Add color pulsing
        float colorPulse = sin(pc.time * 3.0) * 0.1 + 0.9;
        color *= colorPulse;

        // Add subtle rainbow effect
        float rainbow = sin(pc.time * 2.0);
    vec3 rainbowTint = vec3(
    sin(rainbow + 0.0) * 0.1 + 0.9,
    sin(rainbow + 2.0) * 0.1 + 0.9,
    sin(rainbow + 4.0) * 0.1 + 0.9
    );
    color *= rainbowTint;

    return color;
}

// Glow effect
vec3 addGlow(vec2 uv, vec3 color) {
    vec3 glow = vec3(0.0);
    float totalWeight = 0.0;

    for(float i = -2.0; i <= 2.0; i += 1.0) {
        for(float j = -2.0; j <= 2.0; j += 1.0) {
            vec2 offset = vec2(i, j) * 0.005;
            float weight = 1.0 - length(offset) * 0.5;
                if(weight > 0.0) {
                    glow += textureBindless2D(pc.texColor, pc.smpl, uv + offset).rgb * weight;
                    totalWeight += weight;
                }
        }
    }

    glow /= totalWeight;
    return mix(color, glow, 0.3);
}

// Edge detection for dream borders
float detectEdges(vec2 uv) {
    float edge = 0.0;
    vec2 texelSize = 1.0 / textureSize(kTextures2D[pc.texColor], 0);

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            if(i == 0 && j == 0) continue;
            vec2 offset = vec2(i, j) * texelSize;
            vec3 neighborColor = textureBindless2D(pc.texColor, pc.smpl, uv + offset).rgb;
            vec3 centerColor = textureBindless2D(pc.texColor, pc.smpl, uv).rgb;
            edge += length(neighborColor - centerColor);
        }
    }

    return smoothstep(0.2, 0.8, edge);
}

void main() {
    // Apply dream warping
    vec2 warped = dreamWarp(uv);

        // Sample base color with warped coordinates
        vec3 color = textureBindless2D(pc.texColor, pc.smpl, warped).rgb;

    // Apply dream color effects
    color = dreamColor(color);

    // Add glow
    color = addGlow(warped, color);

    // Add edge highlighting
    float edge = detectEdges(warped);
    color = mix(color, vec3(1.0), edge * 0.3);

        // Add vignette
        vec2 vignetteUV = uv * 2.0 - 1.0;
    float vignette = 1.0 - dot(vignetteUV, vignetteUV) * 0.3;
    color *= vignette;

    // Final color adjustment
    color = pow(color, vec3(0.8)); // Slightly brighten

    out_FragColor = vec4(color, 1.0);
}
