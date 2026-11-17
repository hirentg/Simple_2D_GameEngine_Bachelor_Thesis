#include "DebugRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

struct DebugVertex {
    glm::vec3 Position;
    glm::vec4 Color;
};

static const uint32_t MaxDebugVertices = 20000;

struct DebugRendererData {
    GLuint VAO = 0;
    GLuint VBO = 0;

    std::unique_ptr<Shader> LineShader;

    DebugVertex* VertexBufferBase = nullptr;
    DebugVertex* VertexBufferPtr  = nullptr;
    uint32_t     VertexCount = 0;
};

static DebugRendererData s_DebugData;

void DebugRenderer::Init() {
    glGenVertexArrays(1, &s_DebugData.VAO);
    glBindVertexArray(s_DebugData.VAO);

    glGenBuffers(1, &s_DebugData.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, s_DebugData.VBO);
    glBufferData(GL_ARRAY_BUFFER, MaxDebugVertices * sizeof(DebugVertex),
                 nullptr, GL_DYNAMIC_DRAW);

    // Position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),
                          (const void*)offsetof(DebugVertex, Position));
    // Color (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),
                          (const void*)offsetof(DebugVertex, Color));

    s_DebugData.VertexBufferBase = new DebugVertex[MaxDebugVertices];

    s_DebugData.LineShader = std::make_unique<Shader>(
        "assets/shaders/debug_line.vert", "assets/shaders/debug_line.frag");
}

void DebugRenderer::Shutdown() {
    delete[] s_DebugData.VertexBufferBase;
    s_DebugData.VertexBufferBase = nullptr;
    s_DebugData.LineShader.reset();
    glDeleteVertexArrays(1, &s_DebugData.VAO);
    glDeleteBuffers(1, &s_DebugData.VBO);
}

void DebugRenderer::BeginFrame(const OrthographicCamera& camera) {
    s_DebugData.LineShader->Bind();
    s_DebugData.LineShader->SetMat4("u_ViewProjection",
                                     camera.GetViewProjectionMatrix());
    s_DebugData.VertexCount = 0;
    s_DebugData.VertexBufferPtr = s_DebugData.VertexBufferBase;
}

void DebugRenderer::EndFrame() {
    if (s_DebugData.VertexCount == 0) return;

    uint32_t dataSize = static_cast<uint32_t>(
        reinterpret_cast<uint8_t*>(s_DebugData.VertexBufferPtr) -
        reinterpret_cast<uint8_t*>(s_DebugData.VertexBufferBase));

    glBindBuffer(GL_ARRAY_BUFFER, s_DebugData.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize,
                    s_DebugData.VertexBufferBase);

    glBindVertexArray(s_DebugData.VAO);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, s_DebugData.VertexCount);
}

void DebugRenderer::DrawRect(const glm::vec2& min, const glm::vec2& max,
                              const glm::vec4& color)
{
    if(s_DebugData.VertexCount+8>MaxDebugVertices)return;
 // A rectangle is 4 line segments = 8 vertices
    glm::vec2 corners[4] = {
        {min.x, min.y},  // bottom-left
        {max.x, min.y},  // bottom-right
        {max.x, max.y},  // top-right
        {min.x, max.y},  // top-left
    };

    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;

        s_DebugData.VertexBufferPtr->Position = {corners[i].x, corners[i].y, 0.1f};
        s_DebugData.VertexBufferPtr->Color = color;
        s_DebugData.VertexBufferPtr++;

        s_DebugData.VertexBufferPtr->Position = {corners[next].x, corners[next].y, 0.1f};
        s_DebugData.VertexBufferPtr->Color = color;
        s_DebugData.VertexBufferPtr++;

        s_DebugData.VertexCount += 2;
    }
}

void DebugRenderer::DrawLine(const glm::vec2& start, const glm::vec2& end,
                              const glm::vec4& color)
{
    if(s_DebugData.VertexCount+2>MaxDebugVertices)return;
    s_DebugData.VertexBufferPtr->Position = {start.x, start.y, 0.1f};
    s_DebugData.VertexBufferPtr->Color = color;
    s_DebugData.VertexBufferPtr++;

    s_DebugData.VertexBufferPtr->Position = {end.x, end.y, 0.1f};
    s_DebugData.VertexBufferPtr->Color = color;
    s_DebugData.VertexBufferPtr++;

    s_DebugData.VertexCount += 2;
}
