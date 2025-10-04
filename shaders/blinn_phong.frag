#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout (location=0) in vec3 fragPos;
layout (location=1) in vec3 fragNormal;
layout (location=2) in vec2 fragTexCoord;

layout (location=0) out vec4 out_FragColor;

layout(push_constant) uniform PushConstants {
	mat4 mvp;
	uint textureIndex;
} pc;

// Manually define the bindless texture arrays
layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

// Manually define the textureBindless2D function
vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
	return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main() {
	// Use the manually defined textureBindless2D function
	vec4 texColor = textureBindless2D(pc.textureIndex, 0, fragTexCoord);
	out_FragColor = texColor;
}

