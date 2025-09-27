#pragma once

#include <lightweightvk/lvk/LVK.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <optional>
#include <filesystem>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 tangent;
};

struct Material {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float metallicFactor = 0.0f;
    float roughnessFactor = 1.0f;
    lvk::Holder<lvk::TextureHandle> baseColorTexture;
    lvk::Holder<lvk::TextureHandle> normalTexture;
    lvk::Holder<lvk::TextureHandle> metallicRoughnessTexture;
    bool doubleSided = false;
    bool unlit = false;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t materialIndex = 0;
    std::string name;
};

struct Node {
    std::string name;
    glm::mat4 transform = glm::mat4(1.0f);
    std::vector<uint32_t> children;
    std::optional<uint32_t> meshIndex;
    std::optional<uint32_t> cameraIndex;
    std::optional<uint32_t> lightIndex;
};

struct Scene {
    std::string name;
    std::vector<uint32_t> rootNodes;
};

struct GltfAsset {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<Node> nodes;
    std::vector<Scene> scenes;
    std::optional<uint32_t> defaultScene;
    
    // lvk resources
    std::vector<lvk::Holder<lvk::BufferHandle>> vertexBuffers;
    std::vector<lvk::Holder<lvk::BufferHandle>> indexBuffers;
    std::vector<lvk::Holder<lvk::TextureHandle>> textures;
};

class GltfLoader {
public:
    GltfLoader(lvk::IContext* context);
    ~GltfLoader() = default;

    // Load a glTF file and extract all information
    std::optional<GltfAsset> loadGltf(const std::filesystem::path& gltfPath);
    
    // Load a specific glTF asset from fastgltf::Asset
    std::optional<GltfAsset> loadFromAsset(const void* asset, const std::filesystem::path& basePath);

private:
    lvk::IContext* ctx_;
    
    // Helper functions for data extraction
    std::vector<Vertex> extractVertices(const void* asset, const void* primitive);
    std::vector<uint32_t> extractIndices(const void* asset, const void* primitive);
    Material extractMaterial(const void* asset, const void* material, const GltfAsset& gltfAsset);
    lvk::Holder<lvk::TextureHandle> loadTexture(const void* asset, const void* image, const std::filesystem::path& basePath);
    glm::mat4 extractNodeTransform(const void* node);
    
    // Create lvk resources
    lvk::Holder<lvk::BufferHandle> createVertexBuffer(const std::vector<Vertex>& vertices);
    lvk::Holder<lvk::BufferHandle> createIndexBuffer(const std::vector<uint32_t>& indices);
};
