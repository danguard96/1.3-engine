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

vec4[] colors = {vec4(1, 0, 0, 0.75), vec4(0, 1, 0, 0.75), vec4(0, 0, 1, 0.75), vec4(0, 0, 0, 0.5)};
float[] factors = {0.97,0.97,0.97,0.85};

void main() {
    float x = floor(gl_FragCoord.x / 3);
    float y = floor(gl_FragCoord.y / 4);

    vec4 color = textureBindless2D(pc.texColor, pc.smpl, uv);

    color = mix(color * 1.5, color, 1 - (1 / (mod(gl_FragCoord.y * x - (pc.time * 2),256))));

    int i = int(mod(x,4));
    color = mix(colors[i], color, factors[i]);

    color = mix(color * 0.1, color, abs(sin(y + pc.time)));

    out_FragColor = color;
}
