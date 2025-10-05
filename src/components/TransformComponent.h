//
// Created by Yibuz Pokopodrozo on 2025-10-04.
//

#pragma once
#include <components/BaseComponent.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class TransformComponent final : public BaseComponent {
public:
    explicit TransformComponent(BaseComponent* parent_);
    TransformComponent(BaseComponent* parent_, glm::vec3 pos_, glm::quat orientation_, glm::vec3 scale_ = glm::vec3(1.0f, 1.0f, 1.0f));
    ~TransformComponent() override;
    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime_) override;
    void Render() const override;

    [[nodiscard]] glm::vec3 GetPosition() const { return pos; }
    [[nodiscard]] glm::vec3 GetScale() const { return scale; }
    [[nodiscard]] glm::quat GetQuaternion() const { return orientation; }
    [[nodiscard]] glm::mat4 GetTransformMatrix() const;
    void SetTransform(const glm::vec3 pos_, const glm::quat orientation_, const glm::vec3 scale_ = glm::vec3(1.0f, 1.0f, 1.0f) ) {
        pos = pos_;
        orientation = orientation_;
        scale = scale_;
    }

    void SetPosition(const glm::vec3& newPos) { pos = newPos; }
    void SetScale(const glm::vec3& newScale) { scale = newScale; }
    void SetRotation(const glm::quat& newRotation) { orientation = newRotation; }

private:
    glm::vec3 pos{};
    glm::vec3 scale{};
    glm::quat orientation{};
};
