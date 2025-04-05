#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:

    Camera();
    ~Camera();

    // Mark matrices as dirty when camera properties change
    void SetPosition(const glm::vec3& newPosition);
    void SetTarget(const glm::vec3& newTarget) { target = newTarget; isViewDirty = true; }
    void SetUpVector(const glm::vec3& newUpVector) { upVector = newUpVector; isViewDirty = true; }
    void SetAspectRatio(float newAspectRatio) { aspectRatio = newAspectRatio; isProjectionDirty = true; }
    void SetVerticalFOV(float newFOV) { verticalFOV = newFOV; isProjectionDirty = true; }
    void SetNearFar(float newZnear, float newZfar) { znear = newZnear; zfar = newZfar; isProjectionDirty = true; }

    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetTarget() const { return target; }
    const glm::vec3& GetUpVector() const { return upVector; }

    // Update the view matrix if necessary
    const glm::mat4& ViewMatrix() const;

    // Update the projection matrix if necessary
    const glm::mat4& ProjectionMatrix() const;

    bool created = false;

private:

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, -1.0f);
    float aspectRatio = 16.0f / 9.0f; // Default aspect ratio
    float znear = 0.1f;
    float zfar = 1000.0f;
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Y-up by default
    float verticalFOV = 60.0f; // Default FOV

    // Cached matrices
    mutable glm::mat4 cachedViewMatrix;
    mutable glm::mat4 cachedProjectionMatrix;
    mutable bool isViewDirty = true;
    mutable bool isProjectionDirty = true;
};

// Function to create a transformation with position and scale
static glm::mat4 CreateTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
{
    glm::mat4 transform = glm::identity<glm::mat4>();

    transform = glm::translate(transform, translation);

    // Axis-angle rotation
    float angleDegrees = glm::length(rotation);

    if (angleDegrees > 0.0001f)
    {
        glm::vec3 axis = glm::normalize(rotation);
        float angleRadians = glm::radians(angleDegrees);
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angleRadians, axis);
        transform *= rot;
    }

    transform = glm::scale(transform, scale);

    return transform;
}