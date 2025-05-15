#include "Camera.h"

Camera::Camera()
    : position(0.0f, 0.0f, 0.0f),
    target(0.0f, 0.0f, -1.0f),
    aspectRatio(16.0f / 9.0f),
    znear(0.1f),
    zfar(1000.0f),
    upVector(0.0f, 1.0f, 0.0f),
    verticalFOV(60.0f),
    cachedViewMatrix(glm::mat4(0.0f)),
    cachedProjectionMatrix(glm::mat4(0.0f)),
    isViewDirty(true),
    isProjectionDirty(true)
{

}

Camera::~Camera()
{

}

void Camera::SetPosition(const glm::vec3& newPosition) 
{ 
    position = newPosition; 
    isViewDirty = true; 
}

void Camera::SetTarget(const glm::vec3& newTarget) 
{ 
    target = newTarget;
    isViewDirty = true; 
}

void Camera::SetUpVector(const glm::vec3& newUpVector) 
{ 
    upVector = newUpVector; 
    isViewDirty = true; 
}

void Camera::SetAspectRatio(float newAspectRatio) 
{ 
    aspectRatio = newAspectRatio; 
    isProjectionDirty = true; 
}

void Camera::SetVerticalFOV(float newFOV) 
{ 
    verticalFOV = newFOV; 
    isProjectionDirty = true; 
}

void Camera::SetNearFar(float newZnear, float newZfar) 
{ 
    znear = newZnear; 
    zfar = newZfar; 
    isProjectionDirty = true; 
}

const glm::vec3& Camera::GetPosition() const 
{ 
    return position; 
}

const glm::vec3& Camera::GetTarget() const 
{ 
    return target; 
}

const glm::vec3& Camera::GetUpVector() const 
{ 
    return upVector; 
}

const float Camera::GetVerticalFOV() const
{
    return verticalFOV;
}

const float Camera::GetAspectRatio() const
{
    return aspectRatio;
}

const float Camera::GetZNear() const
{
    return znear;
}

const glm::mat4& Camera::ViewMatrix() const
{
    if (isViewDirty)
    {
        cachedViewMatrix = glm::lookAt(position, target, upVector);
        isViewDirty = false;
    }
    return cachedViewMatrix;
}

const glm::mat4& Camera::ProjectionMatrix() const
{
    if (isProjectionDirty)
    {
        cachedProjectionMatrix = glm::perspective(glm::radians(verticalFOV), aspectRatio, znear, zfar);
        isProjectionDirty = false;
    }
    return cachedProjectionMatrix;
}

void CameraMovement(Input& input, Camera& camera, f32 deltaTime)
{
    // Handle camera orbit around origin
    if (input.keys[K_LALT] == BUTTON_PRESSED && input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        float sensitivity = 0.4f;
        float deltaX = input.mouseDelta.x * sensitivity;
        float deltaY = input.mouseDelta.y * sensitivity;

        glm::vec3 position = camera.GetPosition();
        glm::vec3 up = camera.GetUpVector();

        // Convert position (direction from origin) to spherical coordinates
        float radius = glm::length(position);
        float theta = acos(position.y / radius); // Polar angle (from Y-axis)
        float phi = atan2(position.z, position.x); // Azimuthal angle

        // Apply deltas and clamp pitch (theta)
        phi -= glm::radians(deltaX); // Yaw
        theta += glm::radians(deltaY); // Pitch (mouse up = decrease theta)
        theta = glm::clamp(theta, glm::radians(1.0f), glm::radians(179.0f)); // Prevent flipping

        // Convert back to Cartesian coordinates
        glm::vec3 direction;
        direction.x = radius * sin(theta) * cos(phi);
        direction.y = radius * cos(theta);
        direction.z = radius * sin(theta) * sin(phi);

        camera.SetPosition(direction);
        camera.SetTarget(glm::vec3(0.0f));
    }
    // Rotation Logic (look around)
    else if (input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        float sensitivity = 0.4f;
        float deltaX = input.mouseDelta.x * sensitivity;
        float deltaY = input.mouseDelta.y * sensitivity;

        glm::vec3 position = camera.GetPosition();
        glm::vec3 target = camera.GetTarget();
        glm::vec3 up = camera.GetUpVector();

        glm::vec3 forward = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(forward, up));

        // Yaw rotation around up vector
        glm::mat4 yawRot = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaX), up);
        forward = glm::vec3(yawRot * glm::vec4(forward, 0.0f));

        // Pitch rotation around right vector
        glm::mat4 pitchRot = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaY), right);
        forward = glm::vec3(pitchRot * glm::vec4(forward, 0.0f));

        // Clamp pitch to prevent over-rotation
        float pitch = glm::degrees(asin(forward.y));
        float clampedPitch = glm::clamp(pitch, -89.0f, 89.0f); // Limit vertical look

        if (pitch != clampedPitch) {
            glm::vec3 horizontalForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
            forward = horizontalForward * cos(glm::radians(clampedPitch)) + glm::vec3(0.0f, 1.0f, 0.0f) * sin(glm::radians(clampedPitch));
        }

        camera.SetTarget(position + forward);
    }

    // Speed Multiplier
    glm::vec3 position = camera.GetPosition();
    float baseSpeed = 30.0f * deltaTime;
    float speedMultiplier = 4.0f; // Speed boost factor
    float speed = baseSpeed;

    if (input.keys[K_LSHIFT] == BUTTON_PRESSED)
    {
        speed *= speedMultiplier;
    }

    // Camera Vectors
    glm::vec3 forward = glm::normalize(camera.GetTarget() - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.GetUpVector()));
    glm::vec3 up = camera.GetUpVector();

    // Movement
    if (input.keys[K_W] == BUTTON_PRESSED) { position += forward * speed; }
    if (input.keys[K_S] == BUTTON_PRESSED) { position -= forward * speed; }
    if (input.keys[K_A] == BUTTON_PRESSED) { position -= right * speed; }
    if (input.keys[K_D] == BUTTON_PRESSED) { position += right * speed; }
    if (input.keys[K_Q] == BUTTON_PRESSED) { position -= up * speed; }
    if (input.keys[K_E] == BUTTON_PRESSED) { position += up * speed; }

    // Scroll
    if (input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        float zoomSpeed = 50.0f;
        position += forward * input.mouseScrollDeltaY * zoomSpeed * deltaTime;
    }

    camera.SetPosition(position);
    camera.SetTarget(position + forward);
}