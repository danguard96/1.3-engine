#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_FragColor;

layout(push_constant) uniform PushConstants {
    uint texColor;
    uint smpl;
    float time;
    uint noise;
    uint noise2;
} pc;

layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

vec4 fogColor = vec4(1.0);
float noiseScale = 0.5;
float noiseScale2 = 1.0;
float noiseSpeed = 0.24;
float noiseSpeed2 = 0.06;

void main() {
    vec4 color = textureBindless2D(pc.texColor, pc.smpl, uv);
    
    vec2 noiseUV = mod(uv * noiseScale + vec2(pc.time * noiseSpeed, pc.time * noiseSpeed * 0.7), 1);
    vec2 noiseUV2 = mod(uv * noiseScale2 + vec2(pc.time * noiseSpeed2, pc.time * noiseSpeed2 * 0.7), 1);

    float noise = textureBindless2D(pc.noise, pc.smpl, noiseUV).r;
    float noise2 = textureBindless2D(pc.noise2, pc.smpl, noiseUV2).r;

    vec4 color1 = mix(color, fogColor, noise > noise2 ? 1 - (1.5 * noise) : (1 - (1.5 * noise)) / 2);
    vec4 color2 = mix(color, fogColor, noise2 > noise ? 1 - noise2 : noise2);

    color = (color1 + color2) / 2;
    out_FragColor = color;
}