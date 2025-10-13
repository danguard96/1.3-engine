#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_FragColor;

layout(push_constant) uniform PushConstants {
    uint texColor;
    uint smpl;
    float time;
    float _padding;
} pc;

layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
  return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

// 8x8 dithering matrix - initialized with zeros
float[8][8] dither_pattern = {
    {0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0},
    {1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0},
    {0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0},
    {1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0},
    {1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0},
    {0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0},
    {1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0},
    {0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0}
};

void main() {
    vec2 new_uv = vec2(uv.x, uv.y < 0.5 ? 1-uv.y : uv.y);
    vec4 color = textureBindless2D(pc.texColor, pc.smpl, new_uv);

    int kernel_size = 8;
    int pixel_size = 2;

    int x = int(mod((gl_FragCoord.x/pixel_size), kernel_size));
    int y = int(mod((gl_FragCoord.y/pixel_size), kernel_size));

    color = mix(vec4(0.0), color, dither_pattern[x][y]);

    out_FragColor = color;
}
