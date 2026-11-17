#pragma once

#include <glm/glm.hpp>

class OrthographicCamera {
public:
    OrthographicCamera(float aspectRatio, float zoomLevel = 1.0f);

    void SetPosition(const glm::vec2& position);
    void SetRotation(float rotation);
    void SetZoom(float zoom);

    const glm::vec2& GetPosition() const { return m_Position; }
    float GetRotation() const { return m_Rotation; }
    float GetZoom() const { return m_ZoomLevel; }

    const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }

    // Input handling (called from the application layer)
    void OnUpdate(float dt, bool wDown, bool aDown, bool sDown, bool dDown);
    void OnMouseScroll(float yOffset);

private:
    void RecalculateMatrices();

    float m_AspectRatio;
    float m_ZoomLevel;
    float m_CameraSpeed = 5.0f;

    glm::vec2 m_Position = {0.0f, 0.0f};
    float     m_Rotation = 0.0f;

    glm::mat4 m_Projection     = glm::mat4(1.0f);
    glm::mat4 m_View           = glm::mat4(1.0f);
    glm::mat4 m_ViewProjection = glm::mat4(1.0f);

    bool m_Dirty = true;
};
