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

bool perpixel = false;

vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
  return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

vec4[] colors = {vec4(0.035,0.043,0.043, 1), vec4(0.271,0.373,0.337,1), vec4(0.4,0.639,0.451,1), vec4(0.773,0.929,0.675,1),
vec4(0.859,0.996,0.722,1)};

void main() {
    vec4 color = textureBindless2D(pc.texColor, pc.smpl, uv);

    if(perpixel == true){
        color.x = floor((color.x+0.5) * 2)/2;
        color.y = floor((color.y+0.5) * 2)/2;
        color.z = floor((color.z+0.5) * 2)/2;
    }
    else {
        int index = int(floor((((color.x+color.y+color.z)/3)+0.5)*5));
        color = colors[index];
    }

    out_FragColor = color;
}
