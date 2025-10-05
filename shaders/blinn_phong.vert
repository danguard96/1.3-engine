#version 460

layout(push_constant) uniform PushConstants {
	mat4 mvp;
	mat4 model;
	uint textureIndex;
	float _padding[3];
} pc;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 texCoord;

layout (location=0) out vec3 fragPos;
layout (location=1) out vec3 fragNormal;
layout (location=2) out vec2 fragTexCoord;

void main() {
	// Transform to world space for proper lighting calculations
	vec4 worldPos = pc.model * vec4(position, 1.0);
	fragPos = worldPos.xyz;
	
	// Transform normal to world space (inverse transpose of model matrix)
	mat3 normalMatrix = mat3(transpose(inverse(mat3(pc.model))));
	fragNormal = normalize(normalMatrix * normal);
	
	fragTexCoord = texCoord;
	
	// Final position for rasterization
	gl_Position = pc.mvp * vec4(position, 1.0);
}

