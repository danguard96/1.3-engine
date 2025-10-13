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
    out_FragColor = color;
}