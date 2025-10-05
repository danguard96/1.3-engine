//
// Created by Yibuz Pokopodrozo on 2025-10-04.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <components/BaseComponent.h>
#include <glm/glm.hpp>
#include <vector>

class Actor : public BaseComponent {
    std::vector<BaseComponent*> components;

protected:
    glm::mat4 modelMatrix;

public:
    Actor(const Actor &) = delete;
    Actor(Actor &&) = delete;
    Actor &operator=(const Actor &) = delete;
    Actor &operator=(Actor &&) = delete;

    Actor(BaseComponent *parent_): BaseComponent(parent_) {}

    Actor(): BaseComponent(nullptr) {}

    bool OnCreate() override;
    ~Actor() override;
    void OnDestroy() override;
    void Update(float deltaTime) override;
    void Render() const override;

    template<typename ComponentTemplate, typename... Args>
    void AddComponent(Args &&... args_) {
        auto *componentObject = new ComponentTemplate(std::forward<Args>(args_)...);
        components.push_back(componentObject);
    }

    template<typename ComponentTemplate>
    ComponentTemplate* GetComponent() {
        for (BaseComponent* component: components) {
            if (dynamic_cast<ComponentTemplate*>(component) != nullptr) {
                return dynamic_cast<ComponentTemplate*>(component);
            }
        }
        return nullptr;
    }

    template<typename ComponentTemplate>
    void RemoveComponent() {
        for (size_t i = 0; i < components.size(); i++) {
            if (dynamic_cast<ComponentTemplate *>(components[i]) != nullptr) {
                components[i]->OnDestroy();
                delete components[i];
                components.erase(components.begin() + i);
                break;
            }
        }
    }

    glm::mat4 GetModelMatrix();

    void ListComponents();
    void RemoveAllComponents();
};
