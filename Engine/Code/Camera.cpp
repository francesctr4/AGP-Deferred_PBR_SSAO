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
    if (input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        float sensitivity = 0.4f;
        float deltaX = input.mouseDelta.x * sensitivity;
        float deltaY = input.mouseDelta.y * sensitivity;

        glm::vec3 position = camera.GetPosition();
        glm::vec3 target = camera.GetTarget();
        glm::vec3 up = camera.GetUpVector();

        glm::vec3 forward = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(forward, up));

        glm::mat4 yawRot = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaX), up);
        forward = glm::vec3(yawRot * glm::vec4(forward, 0.0f));

        glm::mat4 pitchRot = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaY), right);
        forward = glm::vec3(pitchRot * glm::vec4(forward, 0.0f));

        camera.SetTarget(position + forward);
    }

    glm::vec3 position = camera.GetPosition();
    float baseSpeed = 30.0f * deltaTime;
    float speed = baseSpeed;

    glm::vec3 forward = glm::normalize(camera.GetTarget() - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.GetUpVector()));
    glm::vec3 up = camera.GetUpVector();

    if (input.keys[K_W] == BUTTON_PRESSED)
    {
        position += forward * speed;
    }
    if (input.keys[K_S] == BUTTON_PRESSED)
    {
        position -= forward * speed;
    }

    if (input.keys[K_A] == BUTTON_PRESSED)
    {
        position -= right * speed;
    }

    if (input.keys[K_D] == BUTTON_PRESSED)
    {
        position += right * speed;
    }

    if (input.keys[K_Q] == BUTTON_PRESSED)
    {
        position -= up * speed;
    }

    if (input.keys[K_E] == BUTTON_PRESSED)
    {
        position += up * speed;
    }

    // Scroll
    if (input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        float zoomSpeed = 50.0f;
        position += forward * input.mouseScrollDeltaY * zoomSpeed * deltaTime;
    }

    camera.SetPosition(position);
    camera.SetTarget(position + forward);
}