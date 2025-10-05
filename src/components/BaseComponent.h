//
// Created by Yibuz Pokopodrozo on 2025-10-04.
//

#pragma once

class BaseComponent {
public:
    explicit BaseComponent(BaseComponent* parent_) : parent(parent_), isCreated(false) {}
    virtual ~BaseComponent() = default;
    virtual bool OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void Update(float deltaTime_) = 0;
    virtual void Render() const = 0;

protected:
    BaseComponent* parent;
    /// Just a flag to indicate if the component or actor that inherits this
    /// base class has called OnCreate (true)
    bool isCreated;
};
