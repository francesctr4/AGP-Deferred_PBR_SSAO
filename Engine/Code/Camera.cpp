#include "Camera.h"

// Update the view matrix if necessary

const glm::mat4& Camera::ViewMatrix() const
{
    if (isViewDirty)
    {
        cachedViewMatrix = glm::lookAt(position, target, upVector);
        isViewDirty = false;
    }
    return cachedViewMatrix;
}

// Update the projection matrix if necessary

const glm::mat4& Camera::ProjectionMatrix() const
{
    if (isProjectionDirty)
    {
        cachedProjectionMatrix = glm::perspective(glm::radians(verticalFOV), aspectRatio, znear, zfar);
        isProjectionDirty = false;
    }
    return cachedProjectionMatrix;
}
