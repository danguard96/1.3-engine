//
// Created by Yibuz Pokopodrozo on 2025-10-04.
//

#include <components/TransformComponent.h>

TransformComponent::TransformComponent(BaseComponent* parent_): BaseComponent(parent_) {
    pos = glm::vec3(0.0f, 0.0f, 0.0f);
    orientation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

TransformComponent::TransformComponent(BaseComponent* parent_, glm::vec3 pos_, glm::quat orientation_, glm::vec3 scale_):
    BaseComponent{ parent_ }, pos{ pos_ }, orientation{ orientation_ }, scale{ scale_ } {
}

TransformComponent::~TransformComponent() = default;

bool TransformComponent::OnCreate() {
    if (isCreated == true) return true;
    isCreated = true;
    return true;
}

void TransformComponent::OnDestroy() {}

void TransformComponent::Update(const float deltaTime) {
    // Transform component doesn't need to do anything in update
}

void TransformComponent::Render() const {}

glm::mat4 TransformComponent::GetTransformMatrix() const {
    // 1. Create a matrix that handles only local scale and rotation
    //    (Order: Scale then Rotate on the vertex)
    glm::mat4 local_transform = glm::mat4(1.0f);
    local_transform = glm::scale(local_transform, scale);
    local_transform = local_transform * glm::mat4_cast(orientation);

    // 2. Create a separate translation matrix for the world position
    glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), pos);

    // 3. Combine them: Translate the local transformation into world space.
    //    (Equivalent to: World_Translation_Matrix * Local_Rotation_Scale_Matrix * Vertex)
    return translation_matrix * local_transform;
}
