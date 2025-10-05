//
// Created by Yibuz Pokopodrozo on 2025-10-04.
//

#pragma once
#include <components/BaseComponent.h>
#include <lvk/LVK.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <assimp/scene.h>

struct MeshBuffers {
    lvk::Holder<lvk::BufferHandle> vertexBuffer;
    lvk::Holder<lvk::BufferHandle> indexBuffer;
    uint32_t indexCount;
    
    // Make it movable but not copyable
    MeshBuffers() = default;
    MeshBuffers(const MeshBuffers&) = delete;
    MeshBuffers& operator=(const MeshBuffers&) = delete;
    MeshBuffers(MeshBuffers&&) = default;
    MeshBuffers& operator=(MeshBuffers&&) = default;
};

class MeshComponent : public BaseComponent {
public:
    MeshComponent(BaseComponent* parent_, lvk::IContext* ctx_, const std::string& modelPath_);
    ~MeshComponent() override;
    
    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime_) override;
    void Render() const override;
    
    const std::vector<MeshBuffers>& GetMeshes() const { return meshes; }
    const lvk::Holder<lvk::TextureHandle>& GetTexture() const { return texture; }
    
private:
    lvk::IContext* ctx;
    std::string modelPath;
    std::vector<MeshBuffers> meshes;
    lvk::Holder<lvk::TextureHandle> texture;
    
    bool LoadModel();
    MeshBuffers UploadMesh(const aiMesh* mesh);
    lvk::Holder<lvk::TextureHandle> LoadTexture(const char* fileName);
};
