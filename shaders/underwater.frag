#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_FragColor;

layout(push_constant) uniform PushConstants {
    uint texColor;
    uint smpl;
    float time;
} pc;

// Manually define the bindless texture arrays
layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

// Manually define the textureBindless2D function
vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

vec4 waterColor = vec4(0.0, 0.56, 0.88, 1.0);
float waterIntensity = 0.2;

void main() {
    vec2 mod_uv = uv;
    mod_uv.y = mod_uv.y + sin(((2 * pc.time) + (gl_FragCoord.x / 8))) * 0.002;

    vec4 color = textureBindless2D(pc.texColor, pc.smpl, mod_uv);

    color = mix(color, waterColor, waterIntensity);

    // Make sparkle grid positions move over time
    vec2 movingUV = uv + vec2(pc.time * 0.01, pc.time * 0.005); // Slow drift
    vec2 sparkleGrid = floor(movingUV * vec2(40.0, 16.0));
    float sparkleSeed = dot(sparkleGrid, vec2(12.9898, 78.233));
    float sparkleTime = fract(pc.time * 0.05 + sparkleSeed * 0.1);

    if (sparkleTime > 0.85 && sparkleTime < 0.9) {
        vec2 sparklePos = fract(movingUV * vec2(40.0, 16.0));
        vec2 normalizedPos = sparklePos * vec2(1.0, 40.0/16.0);
        float sparkleDist = distance(normalizedPos, vec2(0.5, 0.5 * 40.0/16.0));
        float sparkleIntensity = (1.0 - sparkleDist * 2.0) * (sparkleTime - 0.85) * 5.0;
        sparkleIntensity = max(0.0, sparkleIntensity);
        color.rgb += vec3(sparkleIntensity * 0.3);
    }
    
    out_FragColor = color;
}