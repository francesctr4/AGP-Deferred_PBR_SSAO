#pragma once

#include "platform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:

    Camera();
    ~Camera();

    // Mark matrices as dirty when camera properties change
    void SetPosition(const glm::vec3& newPosition);
    void SetTarget(const glm::vec3& newTarget);
    void SetUpVector(const glm::vec3& newUpVector);
    void SetAspectRatio(float newAspectRatio);
    void SetVerticalFOV(float newFOV);
    void SetNearFar(float newZnear, float newZfar);

    const glm::vec3& GetPosition() const;
    const glm::vec3& GetTarget() const;
    const glm::vec3& GetUpVector() const;

    const float GetVerticalFOV() const;
    const float GetAspectRatio() const;
    const float GetZNear() const;

    const glm::mat4& ViewMatrix() const;
    const glm::mat4& ProjectionMatrix() const;

    bool created = false;

private:

    glm::vec3 position;
    glm::vec3 target;
    float aspectRatio;
    float znear;
    float zfar;
    glm::vec3 upVector;
    float verticalFOV;

    // Cached matrices
    mutable glm::mat4 cachedViewMatrix;
    mutable glm::mat4 cachedProjectionMatrix;
    mutable bool isViewDirty;
    mutable bool isProjectionDirty;
};

void CameraMovement(Input& input, Camera& camera, f32 deltaTime);