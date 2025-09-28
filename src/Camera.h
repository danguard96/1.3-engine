#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera();
    ~Camera() = default;
    
    void Perspective(float fovy_, float aspectRatio_, float near_, float far_);
    void LookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up);
    
    inline glm::mat4 GetProjectionMatrix() const { return projection; }
    inline glm::mat4 GetViewMatrix() const { return view; }
    glm::mat4 SetViewMatrix(glm::mat4 _view) { return view = _view; }

private:
    glm::mat4 projection;
    glm::mat4 view;
};
