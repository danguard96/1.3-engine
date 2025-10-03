#version 460

layout (location=0) in vec3 fragPos;
layout (location=1) in vec3 fragNormal;
layout (location=2) in vec2 fragTexCoord;
layout (location=0) out vec4 out_FragColor;

// Push constants for lighting
layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 viewPos;
} pc;

// Material properties
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// Light properties
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

void main() {
    // Material properties (you can make these configurable)
    Material material;
    material.ambient = vec3(0.2, 0.2, 0.2);
    material.diffuse = vec3(0.8, 0.8, 0.8);
    material.specular = vec3(1.0, 1.0, 1.0);
    material.shininess = 32.0;
    
    // Light properties
    Light light;
    light.position = pc.lightPos;
    light.ambient = vec3(0.3, 0.3, 0.3);
    light.diffuse = vec3(1.0, 1.0, 1.0);
    light.specular = vec3(1.0, 1.0, 1.0);
    
    // Normalize the normal
    vec3 norm = normalize(fragNormal);
    
    // Calculate light direction
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Calculate view direction
    vec3 viewDir = normalize(pc.viewPos - fragPos);
    
    // Ambient lighting
    vec3 ambient = light.ambient * material.ambient;
    
    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // Blinn-Phong specular lighting
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    // Combine all lighting components
    vec3 result = ambient + diffuse + specular;
    
    out_FragColor = vec4(result, 1.0);
}

