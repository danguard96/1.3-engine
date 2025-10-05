#include <components/CameraComponent.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

CameraComponent::CameraComponent(BaseComponent* parent_)
    : BaseComponent(parent_), 
      position(0.0f, 0.0f, 3.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fovy(45.0f),
      aspectRatio(16.0f / 9.0f),
      nearPlane(0.1f),
      farPlane(1000.0f),
      moveSpeed(5.0f),
      rotationSpeed(2.0f),
      mouseSensitivity(0.01f),
      yaw(-90.0f),
      pitch(0.0f),
      firstMouse(true),
      lastX(0.0f),
      lastY(0.0f),
      viewMatrixDirty(true),
      projectionMatrixDirty(true) {
}

CameraComponent::CameraComponent(BaseComponent* parent_, float fovy_, float aspectRatio_, float near_, float far_)
    : BaseComponent(parent_),
      position(0.0f, 0.0f, 3.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fovy(fovy_),
      aspectRatio(aspectRatio_),
      nearPlane(near_),
      farPlane(far_),
      moveSpeed(5.0f),
      rotationSpeed(2.0f),
      mouseSensitivity(0.1f),
      yaw(-90.0f),
      pitch(0.0f),
      firstMouse(true),
      lastX(0.0f),
      lastY(0.0f),
      viewMatrixDirty(true),
      projectionMatrixDirty(true) {
}

CameraComponent::~CameraComponent() = default;

bool CameraComponent::OnCreate() {
    if (isCreated) return true;
    isCreated = true;
    return true;
}

void CameraComponent::OnDestroy() {
    isCreated = false;
}

void CameraComponent::Update(float deltaTime_) {
    static int updateCount = 0;
    if (++updateCount % 60 == 0) {
        std::cout << "Camera Update called, deltaTime: " << deltaTime_ << std::endl;
    }
    // HandleInput is now called separately in main.cpp with window parameter
    // No need to call HandleInput here anymore
}

void CameraComponent::Render() const {
    // Camera doesn't render anything
}

void CameraComponent::SetPerspective(float fovy_, float aspectRatio_, float near_, float far_) {
    fovy = fovy_;
    aspectRatio = aspectRatio_;
    nearPlane = near_;
    farPlane = far_;
    projectionMatrixDirty = true;
}

void CameraComponent::SetLookAt(const glm::vec3& eye_, const glm::vec3& center_, const glm::vec3& up_) {
    position = eye_;
    target = center_;
    up = up_;
    viewMatrixDirty = true;
}

void CameraComponent::SetPosition(const glm::vec3& position_) {
    position = position_;
    viewMatrixDirty = true;
}

void CameraComponent::SetTarget(const glm::vec3& target_) {
    target = target_;
    viewMatrixDirty = true;
}

glm::mat4 CameraComponent::GetViewMatrix() const {
    if (viewMatrixDirty) {
        UpdateViewMatrix();
    }
    return viewMatrix;
}

glm::mat4 CameraComponent::GetProjectionMatrix() const {
    if (projectionMatrixDirty) {
        UpdateProjectionMatrix();
    }
    return projectionMatrix;
}

glm::mat4 CameraComponent::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

void CameraComponent::UpdateViewMatrix() const {
    viewMatrix = glm::lookAt(position, target, up);
    viewMatrixDirty = false;
}

void CameraComponent::UpdateProjectionMatrix() const {
    projectionMatrix = glm::perspective(glm::radians(fovy), aspectRatio, nearPlane, farPlane);
    projectionMatrixDirty = false;
}

void CameraComponent::HandleInput(float deltaTime) {
    // Get the current window for input
    GLFWwindow* window = glfwGetCurrentContext();
    if (!window) {
        std::cout << "No GLFW window context!" << std::endl;
        return;
    }
    HandleInput(deltaTime, window);
}

void CameraComponent::HandleInput(float deltaTime, GLFWwindow* window) {
    if (!window) {
        std::cout << "No window provided to HandleInput!" << std::endl;
        return;
    }
    
    // Debug: Check if any keys are being pressed
    static bool keyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (!keyPressed) {
            std::cout << "Key pressed detected!" << std::endl;
            keyPressed = true;
        }
    } else {
        keyPressed = false;
    }
    
    // Calculate camera vectors
    glm::vec3 forward = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 upVector = glm::cross(right, forward);
    
    bool moved = false;
    
    // WASD movement - only move position, keep target relative
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += forward * moveSpeed * deltaTime;
        target = position + forward; // Keep target in front
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= forward * moveSpeed * deltaTime;
        target = position + forward; // Keep target in front
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * moveSpeed * deltaTime;
        target = position + forward; // Keep target in front
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right * moveSpeed * deltaTime;
        target = position + forward; // Keep target in front
        moved = true;
    }
    
    // Up/Down movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position -= upVector * moveSpeed * deltaTime;
        target = position + forward; // Keep target in front
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position += upVector * moveSpeed * deltaTime;
        target = position + forward; // Keep target in front
        moved = true;
    }
    
    if (moved) {
        viewMatrixDirty = true;
        // Debug output
        std::cout << "Camera moved to: " << position.x << ", " << position.y << ", " << position.z << std::endl;
    }
    
    // Handle mouse input for camera rotation
    HandleMouseInput(window);
}

void CameraComponent::HandleMouseInput(GLFWwindow* window) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    
    lastX = xpos;
    lastY = ypos;
    
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    // Constrain pitch to prevent screen flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    // Update camera direction based on yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    
    target = position + glm::normalize(direction);
    viewMatrixDirty = true;
}
