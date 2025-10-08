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

const float threshold = 0.8; // Brightness threshold
const float intensity = 2.0; // Bloom strength
const float exposure = 1.2; // Overall brightness
const int count = 64; // Number of samples
const float scale = 0.002; // Scale of the blur samples

float getLuminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 getBrightAreas(vec3 color) {
    float brightness = getLuminance(color);
    return color * smoothstep(threshold - 0.1, threshold + 0.1, brightness);
}

vec3 getBlur(vec2 uv) {
    vec3 blur = vec3(0.0);
    float total_weight = 0.0;

    for (int i = 0; i < count; i++) {
        float angle = float(i) * (2.0 * 3.14159 / float(count));
        float distance = 1.0 - (float(i) / float(count));

        vec2 offset = vec2(cos(angle) * scale * distance, sin(angle) * scale * distance);

        vec3 sample_color = getBrightAreas(textureBindless2D(pc.texColor, pc.smpl, uv + offset).rgb);
        float weight = 1.0 / (1.0 + float(i));

        blur += sample_color * weight;
        total_weight += weight;
    }

    return blur / total_weight;
}

vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 originalColor = textureBindless2D(pc.texColor, pc.smpl, uv).rgb;
    vec3 bloomColor = getBlur(uv);
    vec3 finalColor = originalColor + bloomColor * intensity;
    finalColor *= exposure;
    finalColor = aces(finalColor);
    out_FragColor = vec4(finalColor, 1.0);
}
