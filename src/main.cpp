#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Renderer/Renderer2D.h"
#include "Renderer/DebugRenderer.h"
#include "ECS/Systems/RenderSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Audio/AudioEngine.h"
#include "Debug/ImGuiLayer.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <set>

struct Coin {};
struct Enemy 
{
    float origin; 
};


static EntityID box(Registry& r, glm::vec2 pos, glm::vec2 size, glm::vec4 color, bool fixed)
{
     auto e=r.CreateEntity();
     r.AddComponent<TransformComponent>(e,pos,0.f,size);
     r.AddComponent<SpriteComponent>(e,color, nullptr, glm::vec4(0,0,1,1));
     r.AddComponent<ColliderComponent>(e,size*.5f, glm::vec2(0), uint16_t(1));
     r.AddComponent<RigidbodyComponent>(e, glm::vec2(0), glm::vec2(0), 1.f,0.f, fixed);
     return e;
}


static EntityID load(Registry& r)
{
     std::ifstream f("assets/levels/demo.csv");
     
     if(!f)
         throw std::runtime_error("Missing demo.csv");

     std::string line, cell;
     int y = 0; 
     EntityID player = NULL_ENTITY;

     while (std::getline(f,line))
     {
        std::stringstream row(line);
        int x=0;

        while(std::getline(row,cell,','))
        {
            int id=std::stoi(cell);
            glm::vec2 pos{float(x) - 12, 6.f-y};

            if (id==1)
                box(r,pos,{1,1} , {.19f,.31f,.43f,1}, true);

            if (id==2)
                player = box(r,pos,{.7f,.85f}, {.15f,.85f,.75f,1}, false);

            if (id==3)
            {
                auto e = box(r,pos,{.3f,.3f},{1,.78f,.2f,1},true);
                r.RemoveComponent<RigidbodyComponent>(e);
                r.AddComponent<Coin>(e);
            }
            
            if (id==4)
            {
                auto e = box(r,pos,{.7f,.7f},{1,.3f,.3f,1},true);
                r.AddComponent<Enemy>(e,Enemy{pos.x});
            }

           ++x;
        }++y;
    }
    
     if (player == NULL_ENTITY)
     {
         throw std::runtime_error("Level requires player tile 2");
     }
     return player;

}




static bool overlap(Registry& r, EntityID a, EntityID b)
{
    auto& ta = r.GetComponent<TransformComponent>(a);
    auto& tb = r.GetComponent<TransformComponent>(b);
    auto& ca = r.GetComponent<ColliderComponent>(a);
    auto& cb = r.GetComponent<ColliderComponent>(b);

    return AABBvsAABB(
        ta.position + ca.offset - ca.halfExtents,
        ta.position + ca.offset + ca.halfExtents,
        tb.position + cb.offset - cb.halfExtents,
        tb.position + cb.offset + cb.halfExtents
    ).collided;
}


static int run(GLFWwindow* w, bool smoke)
{
    Renderer2D::Init();
    DebugRenderer::Init();
    ImGuiLayer::Init(w);

    {
        Registry r;
        PhysicsSystem physics;
        AudioEngine audio;

        EntityID player = load(r);
        int score = 0, frames = 0;
        bool paused = false, grounded = false, jumpHeld = false;

        float accumulator = 0;
        double last = glfwGetTime(), simulation = 0;
        std::set<EntityPair> previous;

        auto reset = [&]()
            {
                audio.StopAll();
                r = Registry{};
                player = load(r);

                score = 0;
                simulation = 0;
                accumulator = 0;
                previous.clear();
                grounded = false;
            };

        while (!glfwWindowShouldClose(w))
        {
            double now = glfwGetTime();
            float dt = std::min(float(now - last), .1f);
            last = now;

            glfwPollEvents();
            ImGuiLayer::BeginFrame();

            ImGui::SetNextWindowPos({ 16,16 }, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize({ 330,180 }, ImGuiCond_FirstUseEver);

            ImGui::Begin("Week 7 | Platformer");
            ImGui::Text("Score: %d / 50", score);
            ImGui::TextWrapped("A / D or arrows: move. Space: jump. Collect gold. Avoid red patrols.");

            ImGui::Checkbox("Pause", &paused);
            ImGui::SameLine();

            bool step = ImGui::Button("Step");

            if (ImGui::Button("Restart level"))
                reset();

            ImGui::SameLine();

            if (ImGui::Button("Spawn box"))
                box(r, { 0,4 }, { .7f,.7f }, { .5f,.6f,1,1 }, false);

            if (score >= 50)
                ImGui::Text("Level complete!");

            ImGui::End();

            bool keyboard = !ImGuiLayer::WantsKeyboard();

            auto key = [&](int k)
                {
                    return keyboard && glfwGetKey(w, k) == GLFW_PRESS;
                };

            bool jump = key(GLFW_KEY_SPACE);
            bool request = jump && !jumpHeld;
            jumpHeld = jump;

            if (!paused)
                accumulator += dt;

            if (step)
                accumulator = 1.f / 60;

            while (accumulator >= 1.f / 60)
            {
                simulation += 1. / 60;

                if (r.IsAlive(player) && r.HasComponent<RigidbodyComponent>(player))
                {
                    auto& rb = r.GetComponent<RigidbodyComponent>(player);

                    rb.velocity.x = 6.f * (
                        int(key(GLFW_KEY_D) || key(GLFW_KEY_RIGHT)) -
                        int(key(GLFW_KEY_A) || key(GLFW_KEY_LEFT))
                        );

                    if (request && grounded)
                        rb.velocity.y = 7.5f;
                }

                request = false;

                for (auto e : r.GetView<TransformComponent, Enemy>())
                {
                    r.GetComponent<TransformComponent>(e).position.x =
                        r.GetComponent<Enemy>(e).origin +
                        float(std::sin(simulation)) * 1.5f;
                }

                physics.Update(r, 1.f / 60);
                grounded = false;

                std::set<EntityPair> current;

                for (auto pair : physics.GetCollisionsThisFrame())
                {
                    current.insert(pair);

                    if ((pair.first == player || pair.second == player) && r.IsAlive(player))
                    {
                        auto other = pair.first == player ? pair.second : pair.first;

                        auto& t = r.GetComponent<TransformComponent>(player);
                        auto& o = r.GetComponent<TransformComponent>(other);

                        if (t.position.y > o.position.y &&
                            std::abs(r.GetComponent<RigidbodyComponent>(player).velocity.y) < .1f)
                        {
                            grounded = true;
                        }

                        if (r.HasComponent<Enemy>(other))
                        {
                            t.position = { -10,3 };
                            r.GetComponent<RigidbodyComponent>(player).velocity = { 0,0 };
                        }

                        if (!previous.count(pair))
                            audio.PlaySound("assets/audio/collision.wav", .15f);
                    }
                }

                previous = std::move(current);

                if (r.IsAlive(player) && r.HasComponent<ColliderComponent>(player))
                {
                    for (auto e : r.GetView<TransformComponent, ColliderComponent, Coin>())
                    {
                        if (overlap(r, player, e))
                        {
                            score += 10;
                            r.DestroyEntity(e);
                            audio.PlaySound("assets/audio/coin.wav", .35f);
                        }
                    }

                    auto& t = r.GetComponent<TransformComponent>(player);

                    if (t.position.y < -10)
                    {
                        t.position = { -10,3 };
                        r.GetComponent<RigidbodyComponent>(player).velocity = { 0,0 };
                    }
                }

                r.FlushDestroyedEntities();
                accumulator -= 1.f / 60;
            }

            audio.CollectFinished();

            int width, height;
            glfwGetFramebufferSize(w, &width, &height);

            if (width > 0 && height > 0)
            {
                glViewport(0, 0, width, height);

                OrthographicCamera camera(float(width) / height, 9.f);

                glClearColor(.035f, .055f, .09f, 1);
                glClear(GL_COLOR_BUFFER_BIT);

                Renderer2D::ResetStats();
                Renderer2D::BeginBatch(camera);
                RenderSystem::Update(r);
                Renderer2D::EndBatch();

                DebugRenderer::BeginFrame(camera);

                if (ImGuiLayer::ShowGrid())
                {
                    for (int i = -20; i <= 20; ++i)
                    {
                        DebugRenderer::DrawLine(
                            { float(i * 2),-10 },
                            { float(i * 2),10 },
                            { .2f,.3f,.4f,.4f }
                        );

                        DebugRenderer::DrawLine(
                            { -30,float(i * 2) },
                            { 30,float(i * 2) },
                            { .2f,.3f,.4f,.4f }
                        );
                    }
                }

                if (ImGuiLayer::ShowBoxes())
                {
                    for (auto e : r.GetView<TransformComponent, ColliderComponent>())
                    {
                        auto& t = r.GetComponent<TransformComponent>(e);
                        auto& c = r.GetComponent<ColliderComponent>(e);

                        bool hit = false;

                        for (auto [a, b] : physics.GetCollisionsThisFrame())
                        {
                            if (a == e || b == e)
                                hit = true;
                        }

                        DebugRenderer::DrawRect(
                            t.position + c.offset - c.halfExtents,
                            t.position + c.offset + c.halfExtents,
                            hit ? glm::vec4(1, .2f, .2f, 1) : glm::vec4(.2f, 1, .4f, 1)
                        );
                    }
                }

                DebugRenderer::EndFrame();
            }

            ImGuiLayer::RenderOverlay(r, physics, audio);
            ImGuiLayer::EndFrame();

            r.FlushDestroyedEntities();
            glfwSwapBuffers(w);

            if (smoke && ++frames >= 120)
                break;
        }
    }

    ImGuiLayer::Shutdown();
    DebugRenderer::Shutdown();
    Renderer2D::Shutdown();

    auto error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << error << "\n";
        return 2;
    }

    return 0;
}


int main(int argc, char** argv)
{
    try
    {
        auto executable = std::filesystem::absolute(argv[0]).parent_path();

        if (std::filesystem::exists(executable / "assets"))
            std::filesystem::current_path(executable);

        bool smoke = argc > 1 && std::string(argv[1]) == "--smoke-test";

        glfwSetErrorCallback([](int code, const char* text)
            {
                std::cerr << "GLFW " << code << ": " << text << "\n";
            });

        if (!glfwInit())
            return 1;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        if (smoke)
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        auto* w = glfwCreateWindow(
            1440, 900,
            "Thesis Engine | Week 7",
            nullptr, nullptr
        );

        if (!w)
        {
            glfwTerminate();
            return 1;
        }

        glfwMakeContextCurrent(w);
        glfwSwapInterval(smoke ? 0 : 1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwDestroyWindow(w);
            glfwTerminate();
            return 1;
        }

        std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

        int result = run(w, smoke);

        glfwDestroyWindow(w);
        glfwTerminate();

        return result;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        glfwTerminate();
        return 1;
    }
}
