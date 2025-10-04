#version 460

layout(push_constant) uniform PushConstants {
	mat4 mvp;
} pc;

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 texCoord;

layout (location=0) out vec3 fragPos;
layout (location=1) out vec3 fragNormal;
layout (location=2) out vec2 fragTexCoord;

void main() {
	gl_Position = pc.mvp * vec4(position, 1.0);
	fragPos = position;
	fragNormal = normal;
	fragTexCoord = texCoord;
}

