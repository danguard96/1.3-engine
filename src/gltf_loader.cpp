#include "gltf_loader.h"
#include <iostream>
#include <filesystem>

GltfLoader::GltfLoader(lvk::IContext* context) : ctx_(context) {
}

std::optional<GltfAsset> GltfLoader::loadGltf(const std::filesystem::path& gltfPath) {
    std::cout << "Loading glTF from: " << gltfPath << std::endl;
    
    // For now, return a simple test asset
    GltfAsset asset;
    
    // Create a simple triangle mesh
    Mesh mesh;
    mesh.name = "TestTriangle";
    mesh.vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0, 0, 1}, {0, 0}, {0, 0, 0, 1}},
        {{ 0.5f, -0.5f, 0.0f}, {0, 0, 1}, {1, 0}, {0, 0, 0, 1}},
        {{ 0.0f,  0.5f, 0.0f}, {0, 0, 1}, {0.5, 1}, {0, 0, 0, 1}}
    };
    mesh.indices = {0, 1, 2};
    mesh.materialIndex = 0;
    
    // Create vertex and index buffers
    if (!mesh.vertices.empty()) {
        asset.vertexBuffers.push_back(createVertexBuffer(mesh.vertices));
    }
    if (!mesh.indices.empty()) {
        asset.indexBuffers.push_back(createIndexBuffer(mesh.indices));
    }
    
    asset.meshes.push_back(std::move(mesh));
    
    std::cout << "Created test asset with " << asset.meshes.size() << " meshes" << std::endl;
    return asset;
}

std::optional<GltfAsset> GltfLoader::loadFromAsset(const void* /*asset*/, const std::filesystem::path& /*basePath*/) {
    return std::nullopt;
}

std::vector<Vertex> GltfLoader::extractVertices(const void* /*asset*/, const void* /*primitive*/) {
    return {};
}

std::vector<uint32_t> GltfLoader::extractIndices(const void* /*asset*/, const void* /*primitive*/) {
    return {};
}

Material GltfLoader::extractMaterial(const void* /*asset*/, const void* /*material*/, const GltfAsset& /*gltfAsset*/) {
    Material m;
    return m;
}

lvk::Holder<lvk::TextureHandle> GltfLoader::loadTexture(const void* /*asset*/, const void* /*image*/, const std::filesystem::path& /*basePath*/) {
    return {};
}

glm::mat4 GltfLoader::extractNodeTransform(const void* /*node*/) {
    return glm::mat4(1.0f);
}

lvk::Holder<lvk::BufferHandle> GltfLoader::createVertexBuffer(const std::vector<Vertex>& vertices) {
    if (vertices.empty()) return {};
    lvk::BufferDesc desc{};
    desc.usage = lvk::BufferUsageBits_Vertex;
    desc.storage = lvk::StorageType_Device;
    desc.size = vertices.size() * sizeof(Vertex);
    desc.data = vertices.data();
    desc.debugName = "VertexBuffer";
    return ctx_->createBuffer(desc);
}

lvk::Holder<lvk::BufferHandle> GltfLoader::createIndexBuffer(const std::vector<uint32_t>& indices) {
    if (indices.empty()) return {};
    lvk::BufferDesc desc{};
    desc.usage = lvk::BufferUsageBits_Index;
    desc.storage = lvk::StorageType_Device;
    desc.size = indices.size() * sizeof(uint32_t);
    desc.data = indices.data();
    desc.debugName = "IndexBuffer";
    return ctx_->createBuffer(desc);
}