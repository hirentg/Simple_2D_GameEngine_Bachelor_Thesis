#pragma once

#include <GLFW/glfw3.h>
#include "../ECS/Registry.h"
#include "../ECS/Components.h"
#include "../Renderer/Renderer2D.h"
#include "../Physics/PhysicsSystem.h"
#include "../Audio/AudioEngine.h"

class ImGuiLayer {
public:
    static void Init(GLFWwindow* window);
    static void Shutdown();

    static void BeginFrame();
    static void EndFrame();

    // Render all debug panels
    static void RenderOverlay(Registry& registry,
                               PhysicsSystem& physics,
                               AudioEngine& audioEngine);

    // Whether ImGui wants keyboard/mouse input (game should not process it)
    static bool WantsKeyboard();
    static bool WantsMouse();

 static bool ShowBoxes();
 static bool ShowGrid();
private:
    static void RenderPerformancePanel();
    static void RenderPhysicsPanel(PhysicsSystem& physics);
    static void RenderEntityInspector(Registry& registry);
    static void RenderAudioPanel(AudioEngine& audioEngine);
};
