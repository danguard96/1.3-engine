#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 0) uniform Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

layout(set = 2, binding = 0) uniform Light {
    vec3 position;
    vec3 color;
    float intensity;
} light;

void main() {
    // Normalize the normal
    vec3 normal = normalize(fragNormal);
    
    // Calculate light direction
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Calculate diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * light.intensity * material.diffuse;
    
    // Calculate ambient lighting
    vec3 ambient = 0.1 * light.color * material.ambient;
    
    // Sample texture
    vec3 texColor = texture(texSampler, fragTexCoord).rgb;
    
    // Combine lighting with texture
    vec3 result = (ambient + diffuse) * texColor;
    
    outColor = vec4(result, 1.0);
}
