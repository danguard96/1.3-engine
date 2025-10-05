//
// Created by Yibuz Pokopodrozo on 2025-10-04.
//

#include <components/MeshComponent.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <stdexcept>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

MeshComponent::MeshComponent(BaseComponent* parent_, lvk::IContext* ctx_, const std::string& modelPath_)
    : BaseComponent(parent_), ctx(ctx_), modelPath(modelPath_) {
}

MeshComponent::~MeshComponent() {
    OnDestroy();
}

bool MeshComponent::OnCreate() {
    if (isCreated) return true;
    
    std::cout << "Loading mesh component: " << modelPath << std::endl;
    
    if (!LoadModel()) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        return false;
    }
    
    isCreated = true;
    return true;
}

void MeshComponent::OnDestroy() {
    // LightweightVK handles cleanup automatically
    isCreated = false;
}

void MeshComponent::Update(float deltaTime_) {
    // Mesh component doesn't need to do anything in update
}

void MeshComponent::Render() const {
    // Rendering is handled by the main render loop
}

bool MeshComponent::LoadModel() {
    const aiScene* scene = aiImportFile(modelPath.c_str(), 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    
    if (!scene) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        std::cerr << "Assimp error: " << aiGetErrorString() << std::endl;
        return false;
    }
    
    std::cout << "Model loaded successfully. Meshes: " << scene->mNumMeshes << std::endl;
    
    if (!scene->HasMeshes()) {
        std::cerr << "No meshes found in model: " << modelPath << std::endl;
        aiReleaseImport(scene);
        return false;
    }
    
    // Load all meshes
    for (size_t mi = 0; mi < scene->mNumMeshes; ++mi) {
        try {
            meshes.emplace_back(UploadMesh(scene->mMeshes[mi]));
        } catch (const std::exception& e) {
            std::cerr << "Failed to upload mesh " << mi << ": " << e.what() << std::endl;
        }
    }
    
    // Load texture (simplified - just load the first texture we find)
    if (scene->HasTextures()) {
        // For now, we'll use a placeholder texture loading
        // In a full implementation, you'd extract texture paths from materials
    }
    
    aiReleaseImport(scene);
    return true;
}

MeshBuffers MeshComponent::UploadMesh(const aiMesh* mesh) {
    MeshBuffers out{};

    if (!mesh->HasPositions()) {
        throw std::runtime_error("Mesh has no positions");
    }

    // Extract vertices with position, normal, and texture coordinates
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Extract vertex data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Position
        const aiVector3D pos = mesh->mVertices[i];
        vertex.position = glm::vec3(pos.x, pos.y, pos.z);
        
        // Normal
        if (mesh->HasNormals()) {
            const aiVector3D norm = mesh->mNormals[i];
            vertex.normal = glm::vec3(norm.x, norm.y, norm.z);
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up normal
        }
        
        // Texture coordinates
        if (mesh->HasTextureCoords(0)) {
            const aiVector3D tex = mesh->mTextureCoords[0][i];
            vertex.texCoord = glm::vec2(tex.x, tex.y);
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f); // Default UV
        }
        
        vertices.push_back(vertex);
    }
    
    // Extract indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        for (int j = 0; j < 3; j++) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    
    out.indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers with optimized data upload
    out.vertexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Vertex,
        .storage = lvk::StorageType_Device,
        .size = sizeof(Vertex) * vertices.size(),
        .data = vertices.data(),
        .debugName = "Buffer: vertex"
    });
    
    out.indexBuffer = ctx->createBuffer({
        .usage = lvk::BufferUsageBits_Index,
        .storage = lvk::StorageType_Device,
        .size = sizeof(uint32_t) * indices.size(),
        .data = indices.data(),
        .debugName = "Buffer: index"
    });

    return out;
}

lvk::Holder<lvk::TextureHandle> MeshComponent::LoadTexture(const char* fileName) {
    int width, height, channels;
    unsigned char* data = stbi_load(fileName, &width, &height, &channels, 4); // Force RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << fileName << std::endl;
        return {};
    }
    
    auto texture = ctx->createTexture({
        .type = lvk::TextureType_2D,
        .format = lvk::Format_RGBA_SRGB8, // Use sRGB for color textures
        .dimensions = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = lvk::TextureUsageBits_Sampled,
        .data = data,
        .debugName = fileName
    });
    
    stbi_image_free(data);
    return texture;
}
