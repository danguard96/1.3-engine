#pragma once
#include <components/BaseComponent.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declaration
struct GLFWwindow;

class CameraComponent final : public BaseComponent {
public:
    CameraComponent(BaseComponent* parent_);
    CameraComponent(BaseComponent* parent_, float fovy_, float aspectRatio_, float near_, float far_);
    ~CameraComponent() override;
    
    bool OnCreate() override;
    void OnDestroy() override;
    void Update(float deltaTime_) override;
    void Render() const override;
    
    // Camera control methods
    void SetPerspective(float fovy_, float aspectRatio_, float near_, float far_);
    void SetLookAt(const glm::vec3& eye_, const glm::vec3& center_, const glm::vec3& up_);
    void SetPosition(const glm::vec3& position_);
    void SetTarget(const glm::vec3& target_);
    
    // Get camera matrices
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewProjectionMatrix() const;
    
    // Camera properties
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetTarget() const { return target; }
    glm::vec3 GetUp() const { return up; }
    
    // Input handling
    void HandleInput(float deltaTime);
    void HandleInput(float deltaTime, GLFWwindow* window);
    void HandleMouseInput(GLFWwindow* window);
    
private:
    // Camera properties
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    
    // Projection properties
    float fovy;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    
    // Movement properties
    float moveSpeed;
    float rotationSpeed;
    float mouseSensitivity;
    float yaw;
    float pitch;
    bool firstMouse;
    float lastX;
    float lastY;
    
    // Cached matrices
    mutable glm::mat4 viewMatrix;
    mutable glm::mat4 projectionMatrix;
    mutable bool viewMatrixDirty;
    mutable bool projectionMatrixDirty;
    
    void UpdateViewMatrix() const;
    void UpdateProjectionMatrix() const;
};
