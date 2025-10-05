#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout (location=0) in vec3 fragPos;
layout (location=1) in vec3 fragNormal;
layout (location=2) in vec2 fragTexCoord;

layout (location=0) out vec4 out_FragColor;

layout(push_constant) uniform PushConstants {
	mat4 mvp;
	mat4 model;
	uint textureIndex;
	float _padding[3];
} pc;

// Manually define the bindless texture arrays
layout (set = 0, binding = 0) uniform texture2D kTextures2D[];
layout (set = 0, binding = 1) uniform sampler kSamplers[];

// Manually define the textureBindless2D function
vec4 textureBindless2D(uint textureid, uint samplerid, vec2 uv) {
	return texture(nonuniformEXT(sampler2D(kTextures2D[textureid], kSamplers[samplerid])), uv);
}

void main() {
	// Sample the texture
	vec4 texColor = textureBindless2D(pc.textureIndex, 0, fragTexCoord);
	
	// Lighting setup - these are in world space
	vec3 lightPos = vec3(2.0, 2.0, 2.0);  // Fixed light position in world space
	vec3 viewPos = vec3(0.0, 0.0, 3.0);    // Camera position in world space
	vec3 lightColor = vec3(2.0, 2.0, 2.0); // Bright light
	float shininess = 64.0;
	
	// Use world space fragment position and normal (already transformed in vertex shader)
	vec3 norm = normalize(fragNormal);
	vec3 worldPos = fragPos;
	
	// Calculate light direction from world space
	vec3 lightDir = normalize(lightPos - worldPos);
	
	// Calculate view direction from world space
	vec3 viewDir = normalize(viewPos - worldPos);
	
	// Calculate halfway vector for Blinn-Phong
	vec3 halfwayDir = normalize(lightDir + viewDir);
	
	// Ambient lighting
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * lightColor;
	
	// Diffuse lighting
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	// Specular lighting (Blinn-Phong)
	float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
	vec3 specular = spec * lightColor;
	
	// Combine all lighting components
	vec3 lighting = ambient + diffuse + specular;
	vec3 result = lighting * texColor.rgb;
	
	out_FragColor = vec4(result, texColor.a);
}

