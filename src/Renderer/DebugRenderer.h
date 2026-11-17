#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"
#include "OrthographicCamera.h"
#include <vector>
#include <memory>

// Renders wireframe debug shapes (collision boxes, grid lines) on top of
// the game scene. Uses GL_LINES instead of filled triangles.
class DebugRenderer {
public:
    static void Init();
    static void Shutdown();

    static void BeginFrame(const OrthographicCamera& camera);
    static void EndFrame();

    // Draw a wireframe rectangle (AABB outline)
    static void DrawRect(const glm::vec2& min, const glm::vec2& max,
                         const glm::vec4& color);

    // Draw a line segment
    static void DrawLine(const glm::vec2& start, const glm::vec2& end,
                         const glm::vec4& color);
};
