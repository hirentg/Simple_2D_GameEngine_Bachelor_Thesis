#include "ImGuiLayer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>

// ── Rolling FPS history for graph ──────────────────────────────────────────────
static float s_FpsHistory[120] = {};
static int   s_FpsHistoryIndex = 0;

// ── Debug rendering toggles ───────────────────────────────────────────────────
static bool s_ShowCollisionBoxes = false;
static bool s_ShowSpatialGrid   = false;

// ── Audio state ────────────────────────────────────────────────────────────────
static uint32_t s_BgmHandle = 0;
static float    s_BgmVolume = 0.5f;

void ImGuiLayer::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Dark theme with custom accent colors
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding  = 4.0f;
    style.GrabRounding   = 4.0f;
    style.WindowPadding  = ImVec2(10, 10);
    style.FramePadding   = ImVec2(6, 4);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiLayer::WantsKeyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiLayer::WantsMouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

// ── Main overlay entry point ───────────────────────────────────────────────────
void ImGuiLayer::RenderOverlay(Registry& registry,
                                PhysicsSystem& physics,
                                AudioEngine& audioEngine)
{
    RenderPerformancePanel();
    RenderPhysicsPanel(physics);
    RenderEntityInspector(registry);
    RenderAudioPanel(audioEngine);
}

// ── Performance Panel ──────────────────────────────────────────────────────────
void ImGuiLayer::RenderPerformancePanel() {
    ImGui::SetNextWindowPos(ImVec2(1010,16), ImGuiCond_FirstUseEver);
 ImGui::SetNextWindowSize(ImVec2(390,190), ImGuiCond_FirstUseEver);
 ImGui::Begin("Performance");

    float fps = ImGui::GetIO().Framerate;
    float frameTime = 1000.0f / fps;

    // Update rolling FPS graph
    s_FpsHistory[s_FpsHistoryIndex] = fps;
    s_FpsHistoryIndex = (s_FpsHistoryIndex + 1) % 120;

    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.2f ms", frameTime);
    ImGui::PlotLines("##FPS", s_FpsHistory, 120, s_FpsHistoryIndex,
                     nullptr, 0.0f, 120.0f, ImVec2(0, 50));

    ImGui::Separator();
    Renderer2DStats stats = Renderer2D::GetStats();
    ImGui::Text("Draw Calls: %u", stats.DrawCalls);
    ImGui::Text("Quads: %u", stats.QuadCount);

    ImGui::End();
}

// ── Physics Debug Panel ────────────────────────────────────────────────────────
void ImGuiLayer::RenderPhysicsPanel(PhysicsSystem& physics) {
    ImGui::SetNextWindowPos(ImVec2(1010,220), ImGuiCond_FirstUseEver);
 ImGui::SetNextWindowSize(ImVec2(390,180), ImGuiCond_FirstUseEver);
 ImGui::Begin("Physics");

    ImGui::DragFloat2("Gravity", &physics.Gravity.x, 0.1f, -50.0f, 50.0f);

    ImGui::Separator();
    ImGui::Checkbox("Show Collision Boxes", &s_ShowCollisionBoxes);
    ImGui::Checkbox("Show Spatial Grid", &s_ShowSpatialGrid);

    ImGui::Separator();
    ImGui::Text("Collisions this frame: %zu",
                physics.GetCollisionsThisFrame().size());

    ImGui::End();
}

// ── Entity Inspector ───────────────────────────────────────────────────────────
void ImGuiLayer::RenderEntityInspector(Registry& registry) {
    ImGui::SetNextWindowPos(ImVec2(1010,414), ImGuiCond_FirstUseEver);
 ImGui::SetNextWindowSize(ImVec2(390,290), ImGuiCond_FirstUseEver);
 ImGui::Begin("Entity Inspector");

    ImGui::Text("Entity Count: %zu", registry.EntityCount());
    ImGui::Separator();

    if (ImGui::Button("+ Create Entity")) {
        EntityID e = registry.CreateEntity();
        registry.AddComponent<TransformComponent>(e);
        registry.AddComponent<SpriteComponent>(e);
    }

    ImGui::Separator();

    // Iterate over entities with TransformComponent (most entities have one)
    auto view = registry.GetView<TransformComponent>();
    for (EntityID entity : view) {
        std::string label = "Entity " + std::to_string(entity);

        ImGui::PushID(static_cast<int>(entity));
 if (ImGui::TreeNode(label.c_str())) {
            // Transform
            if (registry.HasComponent<TransformComponent>(entity)) {
                auto& tf = registry.GetComponent<TransformComponent>(entity);
                ImGui::DragFloat2("Position", &tf.position.x, 0.1f);
                ImGui::DragFloat("Rotation", &tf.rotation, 1.0f, 0.0f, 360.0f);
                ImGui::DragFloat2("Scale", &tf.scale.x, 0.05f, 0.1f, 10.0f);
            }

            // Sprite
            if (registry.HasComponent<SpriteComponent>(entity)) {
                auto& sprite = registry.GetComponent<SpriteComponent>(entity);
                ImGui::ColorEdit4("Tint Color", &sprite.color.x);
            }

            // Rigidbody
            if (registry.HasComponent<RigidbodyComponent>(entity)) {
                auto& rb = registry.GetComponent<RigidbodyComponent>(entity);
                ImGui::Text("Velocity: (%.2f, %.2f)", rb.velocity.x, rb.velocity.y);
                ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.1f, 100.0f);
                ImGui::SliderFloat("Restitution", &rb.restitution, 0.0f, 1.0f);
                ImGui::Checkbox("Static", &rb.isStatic);
            }

            // Collider
            if (registry.HasComponent<ColliderComponent>(entity)) {
                auto& col = registry.GetComponent<ColliderComponent>(entity);
                ImGui::DragFloat2("Half Extents", &col.halfExtents.x, 0.05f,
                                  0.05f, 10.0f);
                ImGui::DragFloat2("Offset", &col.offset.x, 0.05f);
            }

            ImGui::Spacing();
            if (ImGui::Button(("Destroy##" + std::to_string(entity)).c_str())) {
                registry.DestroyEntity(entity);
            }

            ImGui::TreePop();
        }
 ImGui::PopID();
    }

    ImGui::End();
}

// ── Audio Panel ────────────────────────────────────────────────────────────────
void ImGuiLayer::RenderAudioPanel(AudioEngine& audioEngine) {
    ImGui::SetNextWindowPos(ImVec2(1010,718), ImGuiCond_FirstUseEver);
 ImGui::SetNextWindowSize(ImVec2(390,165), ImGuiCond_FirstUseEver);
 ImGui::Begin("Audio");

    ImGui::Text("Background Music");
    if (ImGui::Button("Play BGM")) {
        if (s_BgmHandle == 0) {
            s_BgmHandle = audioEngine.PlaySound(
                "assets/audio/bgm.wav", s_BgmVolume, true);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop BGM")) {
        audioEngine.StopSound(s_BgmHandle);
        s_BgmHandle = 0;
    }
    if (ImGui::SliderFloat("BGM Volume", &s_BgmVolume, 0.0f, 1.0f)) {
        audioEngine.SetMasterVolume(s_BgmVolume);
    }

    ImGui::Separator();
    ImGui::Text("Sound Effects");
    if (ImGui::Button("Test Collision SFX")) {
        audioEngine.PlaySound("assets/audio/collision.wav", 0.8f, false);
    }

    ImGui::End();
}

bool ImGuiLayer::ShowBoxes(){return s_ShowCollisionBoxes;}
bool ImGuiLayer::ShowGrid(){return s_ShowSpatialGrid;}
