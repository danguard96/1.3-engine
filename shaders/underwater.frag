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

layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

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

    vec2 movingUV = uv + vec2(pc.time * 0.01, pc.time * 0.005);
    vec2 sparkleGrid = floor(movingUV * vec2(40.0, 16.0));
    float sparkleSeed = dot(sparkleGrid, vec2(12.9898, 78.233));
    float sparkleTime = fract(pc.time * 0.05 + sparkleSeed * 0.1);

    if (sparkleTime > 0.85 && sparkleTime < 0.9) {
        vec2 sparklePos = fract(movingUV * vec2(40.0, 16.0));
        vec2 normalizedPos = sparklePos * vec2(1.0, 40.0/16.0);
        float sparkleDist = distance(normalizedPos, vec2(0.5, 0.5 * 40.0/16.0));
        float sparkleIntensity = (1.0 - sparkleDist * 2.0) * (sparkleTime - 0.85) * 5.0;
        sparkleIntensity = max(0.0, sparkleIntensity);
        color.rgb += vec3(sparkleIntensity * 0.5);
    }
    
    for (int i = 0; i < 8; i++) {
        float bubbleSpeed = 0.1 + float(i) * 0.05;
        float bubbleY = 1.0 - fract(pc.time * bubbleSpeed + float(i) * 0.33);
        
        float randomX = fract(float(i) * 0.618033988749895);
        vec2 bubblePos = vec2(randomX, bubbleY);
        
        float sizeSeed = float(i) * 7.891;
        float randomSize = fract(sin(sizeSeed) * 12345.6789) * 0.08;
        float bubbleRadius = max(0.02, randomSize);
        
        vec2 aspectCorrectedUV = uv * vec2(1.0, 0.6);
        vec2 aspectCorrectedBubblePos = bubblePos * vec2(1.0, 0.6);
        
        float bubbleDist = distance(aspectCorrectedUV, aspectCorrectedBubblePos);
        if (bubbleDist < bubbleRadius) {
            float bubbleStrength = (1.0 - bubbleDist / bubbleRadius) * 0.2;
            mod_uv += (uv - bubblePos) * bubbleStrength;
            
            float bubbleAlpha = 1.0 - smoothstep(bubbleRadius * 0.6, bubbleRadius, bubbleDist);
            vec3 bubbleColor = vec3(0.8, 0.9, 1.0);
            float bubbleHighlight = 1.0 - smoothstep(0.0, 0.03, bubbleDist);
            bubbleColor += vec3(bubbleHighlight * 0.3);
            
            color.rgb = mix(color.rgb, bubbleColor, bubbleAlpha * 0.2);
        }
    }
    
    vec4 bubbleColor = textureBindless2D(pc.texColor, pc.smpl, mod_uv);
    color = mix(color, bubbleColor, 0.7);

    out_FragColor = color;
}