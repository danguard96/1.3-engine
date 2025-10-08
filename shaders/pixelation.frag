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

float pixelation = 140.0;

void main() {
    vec2 pixels = vec2(pixelation);
    vec2 pixelated = floor(uv * pixels) / pixels;
    vec4 color = textureBindless2D(pc.texColor, pc.smpl, pixelated);
    out_FragColor = color;
}
