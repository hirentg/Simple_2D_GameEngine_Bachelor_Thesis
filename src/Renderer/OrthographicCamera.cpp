#include "OrthographicCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

OrthographicCamera::OrthographicCamera(float aspectRatio, float zoomLevel)
    : m_AspectRatio(aspectRatio), m_ZoomLevel(zoomLevel)
{
    RecalculateMatrices();
}

void OrthographicCamera::SetPosition(const glm::vec2& position) {
    m_Position = position;
    m_Dirty = true;
}

void OrthographicCamera::SetRotation(float rotation) {
    m_Rotation = rotation;
    m_Dirty = true;
}

void OrthographicCamera::SetZoom(float zoom) {
    m_ZoomLevel = std::clamp(zoom, 0.25f, 10.0f);
    m_Dirty = true;
}

void OrthographicCamera::OnUpdate(float dt, bool wDown, bool aDown, bool sDown, bool dDown) {
    float velocity = m_CameraSpeed * m_ZoomLevel * dt;

    if (aDown) { m_Position.x -= velocity; m_Dirty = true; }
    if (dDown) { m_Position.x += velocity; m_Dirty = true; }
    if (sDown) { m_Position.y -= velocity; m_Dirty = true; }
    if (wDown) { m_Position.y += velocity; m_Dirty = true; }

    if (m_Dirty) {
        RecalculateMatrices();
    }
}

void OrthographicCamera::OnMouseScroll(float yOffset) {
    m_ZoomLevel -= yOffset * 0.25f;
    m_ZoomLevel = std::clamp(m_ZoomLevel, 0.25f, 10.0f);
    m_Dirty = true;
    RecalculateMatrices();
}

void OrthographicCamera::RecalculateMatrices() {
    float left   = -m_AspectRatio * m_ZoomLevel;
    float right  =  m_AspectRatio * m_ZoomLevel;
    float bottom = -m_ZoomLevel;
    float top    =  m_ZoomLevel;

    m_Projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

    glm::mat4 transform = glm::translate(glm::mat4(1.0f),
                                          glm::vec3(m_Position, 0.0f));
    if (m_Rotation != 0.0f) {
        transform = glm::rotate(transform, glm::radians(m_Rotation),
                                glm::vec3(0.0f, 0.0f, 1.0f));
    }

    m_View = glm::inverse(transform);
    m_ViewProjection = m_Projection * m_View;
    m_Dirty = false;
}
