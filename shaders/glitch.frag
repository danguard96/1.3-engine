#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_FragColor;

layout(push_constant) uniform PushConstants {
  uint texColor;
  uint smpl;
} pc;

// Manually define the bindless texture arrays
layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

// Manually define the textureBindless2D function
vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
  return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

// Time-based animation
float time = float(uint(gl_FragCoord.x + gl_FragCoord.y) % 6311) / 6311.0;

// Random function
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// RGB split with random offset
vec3 rgbSplitGlitch(vec2 uv) {
    float offset = (rand(vec2(time, uv.y)) - 0.5) * 0.05;
    
    vec3 col;
    col.r = textureBindless2D(pc.texColor, pc.smpl, vec2(uv.x + offset, uv.y)).r;
    col.g = textureBindless2D(pc.texColor, pc.smpl, vec2(uv.x, uv.y)).g;
    col.b = textureBindless2D(pc.texColor, pc.smpl, vec2(uv.x - offset, uv.y)).b;
    return col;
}

// Block glitch effect
vec2 blockGlitch(vec2 uv) {
    float block = floor(uv.y * 10.0);
    float noise = rand(vec2(block, time));
    
    if (noise > 0.95) {
        uv.x += (noise - 0.95) * 2.0;
    }
    
    return uv;
}

// Scanline effect with distortion
float scanline(vec2 uv) {
    float wave = sin(uv.y * 400.0 + time * 10.0) * 0.5 + 0.5;
    float scanline = step(0.5, wave) * 0.15;
    return 1.0 - scanline;
}

void main() {
    // Apply block glitch to UV coordinates
    vec2 glitchUV = blockGlitch(uv);
    
    // Get color with RGB split
    vec3 color = rgbSplitGlitch(glitchUV);
    
    // Add scanlines
    color *= scanline(glitchUV);
    
    // Random color shift
    if (rand(vec2(time, glitchUV.x)) > 0.99) {
        color = vec3(1.0) - color;
    }
    
    // Add vertical noise bars
    float noise_x = floor(glitchUV.x * 20.0);
    if (rand(vec2(noise_x, time)) > 0.98) {
        color *= 0.5;
    }
    
    // Add random bright pixels
    if (rand(glitchUV + time) > 0.995) {
        color = vec3(1.0);
    }
    
    // Add wave distortion
    float wave = sin(glitchUV.y * 10.0 + time * 5.0) * 0.1;
    color *= (1.0 + wave);
    
    out_FragColor = vec4(color, 1.0);
}
